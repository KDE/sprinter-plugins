/*
 *   Copyright (C) 2011 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "activities.h"

#include <QCoreApplication>

#include <KI18n/KLocalizedString>

KActivitiesProxy::KActivitiesProxy()
    : QObject(),
      m_activities(0)
{
    qRegisterMetaType<KActivities::Consumer::ServiceStatus>("KActivities::Consumer::ServiceStatus");
}

void KActivitiesProxy::start()
{
//     qDebug() << "Running the KAMD proxy" << thread() << QCoreApplication::instance()->thread();
    m_activities = new KActivities::Controller(this);
    connect(m_activities, &KActivities::Consumer::serviceStatusChanged,
            this, &KActivitiesProxy::serviceStatusChanged);
    connect(m_activities, &KActivities::Consumer::activitiesChanged,
            this, &KActivitiesProxy::activitiesChanged);
    connect(m_activities, &KActivities::Consumer::currentActivityChanged,
            this, &KActivitiesProxy::currentActivityChanged);
    emit activitiesChanged(m_activities->activities());
    emit currentActivityChanged(m_activities->currentActivity());
    emit serviceStatusChanged(m_activities->serviceStatus());
}

QFuture<bool> KActivitiesProxy::setCurrentActivity(const QString &activityId)
{
    return m_activities->setCurrentActivity(activityId);
}

ActivitySessionData::ActivitySessionData(Sprinter::Runner *runner)
    : Sprinter::RunnerSessionData(runner),
      isEnabled(false),
      m_activitiesProxy(new KActivitiesProxy)
{
    m_activitiesProxy->moveToThread(QCoreApplication::instance()->thread());
    connect(m_activitiesProxy,
            &KActivitiesProxy::serviceStatusChanged,
            this,
            &ActivitySessionData::serviceStatusChanged);
    connect(m_activitiesProxy,
            &KActivitiesProxy::activitiesChanged,
            this,
            &ActivitySessionData::activitiesChanged);
    connect(m_activitiesProxy,
            &KActivitiesProxy::currentActivityChanged,
            this,
            &ActivitySessionData::currentActivityChanged);
    QMetaObject::invokeMethod(m_activitiesProxy, "start");
}

ActivitySessionData::~ActivitySessionData()
{
    QMetaObject::invokeMethod(m_activitiesProxy, "deleteLater");
}

void ActivitySessionData::serviceStatusChanged(KActivities::Consumer::ServiceStatus status)
{
    isEnabled = status == KActivities::Consumer::Running;
//     qDebug() << "$$$$$$$$$$$$$$$$ ENABLED?????" << isEnabled;
}

void ActivitySessionData::activitiesChanged(const QStringList &acts)
{
    activities = acts;
    qSort(activities);
//     qDebug() << "$$$$$$$$$$$$$$$$ ?????" << activities;
}

void ActivitySessionData::currentActivityChanged(const QString &id)
{
    currentActivity = id;
//     qDebug() << "$$$$$$$$$$$$$$$$ ?????" << currentActivity;
}

bool ActivitySessionData::setCurrentActivity(const QString &activityId)
{
    QFuture<bool> rv;
    QMetaObject::invokeMethod(m_activitiesProxy, "setCurrentActivity",
                              Qt::BlockingQueuedConnection,
                              Q_RETURN_ARG(QFuture<bool>, rv),
                              Q_ARG(QString, activityId));
    rv.waitForFinished();

    //TODO: this shows one of the weaknesses in the QFuture based APIs ....
    bool value = rv.results().isEmpty() ? false : rv.result();
    return value;
}

ActivityRunner::ActivityRunner(QObject *parent)
    : Sprinter::Runner(parent),
      m_keywordi18n(i18nc("Search keyword", "activity")),
      m_keyword("activity"),
      m_defaultIcon(QIcon::fromTheme("preferences-activities"))
{
}

Sprinter::RunnerSessionData *ActivityRunner::createSessionData()
{
    return new ActivitySessionData(this);
}

void ActivityRunner::match(Sprinter::MatchData &matchData)
{
    ActivitySessionData *sessionData = qobject_cast<ActivitySessionData *>(matchData.sessionData());
    if (!sessionData  || !sessionData->isEnabled) {
        return;
    }

    const QString term = matchData.queryContext().query();
    bool list = false;
    bool triggerWord = false;
    QString name;

    if (matchData.queryContext().isDefaultMatchesRequest()) {
        list = true;
    } else if (term.startsWith(m_keywordi18n, Qt::CaseInsensitive)) {
        triggerWord = true;
        if (term.size() == m_keywordi18n.size()) {
            list = true;
        } else {
            name = term.right(term.size() - m_keywordi18n.size()).trimmed();
            list = name.isEmpty();
        }
    } else if (term.startsWith(m_keyword, Qt::CaseInsensitive)) {
        triggerWord = true;
        if (term.size() == m_keyword.size()) {
            list = true;
        } else {
            name = term.right(term.size() - m_keyword.size()).trimmed();
            list = name.isEmpty();
        }
    } else {
        name = term;
    }

    const QStringList activities = sessionData->activities;
    const QString currentActivity = sessionData->currentActivity;

    if (!matchData.isValid()) {
        return;
    }

    if (list) {
        foreach (const QString &activity, activities) {
            if (currentActivity == activity) {
                continue;
            }

            KActivities::Info info(activity);
            addMatch(info,
                     (info.state() == KActivities::Info::Running ||
                      info.state() == KActivities::Info::Starting) ?
                        Sprinter::QuerySession::ExactMatch :
                        Sprinter::QuerySession::CloseMatch,
                     matchData);

            if (!matchData.isValid()) {
                return;
            }
        }
    } else {
        foreach (const QString &activity, activities) {
            if (currentActivity == activity) {
                continue;
            }

            KActivities::Info info(activity);
            if (info.name().startsWith(name, Qt::CaseInsensitive)) {
                bool exact = info.name().compare(name, Qt::CaseInsensitive) == 0;
                addMatch(info,
                         triggerWord ? (exact ?
                                            Sprinter::QuerySession::ExactMatch :
                                            Sprinter::QuerySession::CloseMatch)
                                     : (exact ?
                                            Sprinter::QuerySession::CloseMatch :
                                            Sprinter::QuerySession::FuzzyMatch),
                         matchData);
            }

            if (!matchData.isValid()) {
                return;
            }
        }
    }
}

void ActivityRunner::addMatch(const KActivities::Info &activity,
                              Sprinter::QuerySession::MatchPrecision precision,
                              Sprinter::MatchData &matchData)
{
    Sprinter::QueryMatch match;
    match.setData(activity.id());
    match.setType(Sprinter::QuerySession::ActivityType);
    match.setSource(Sprinter::QuerySession::FromLocalService);
    match.setImage(image(activity, matchData.queryContext()));
    match.setTitle(i18n("Switch to activity \"%1\"", activity.name()));
    match.setPrecision(precision);
    matchData << match;
}

QImage ActivityRunner::image(const KActivities::Info &activity,
                             const Sprinter::QueryContext &context)
{
    if (activity.icon().isEmpty()) {
        return generateImage(m_defaultIcon, context);
    }

    QIcon icon;
    if (activity.icon().startsWith('/')) {
        icon = QIcon(activity.icon());
    } else {
        icon = QIcon::fromTheme(activity.icon());
    }

    return generateImage(icon, context);
}

bool ActivityRunner::exec(const Sprinter::QueryMatch &match)
{
    ActivitySessionData *sessionData = qobject_cast<ActivitySessionData *>(match.sessionData());
    if (!sessionData  || !sessionData->isEnabled) {
        return false;
    }

    return sessionData->setCurrentActivity(match.data().toString());
}

#include "moc_activities.cpp"
