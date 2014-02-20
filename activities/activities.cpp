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

#include <QEvent>
#include <QTimer>
ActivitySessionData::ActivitySessionData(Sprinter::Runner *runner)
    : Sprinter::RunnerSessionData(runner),
//       activities(new KActivities::Controller(this)),
      activities(0),
      isEnabled(false)
{
    qDebug() << "Creating in thread" << thread();
            activities = new KActivities::Controller(this);
    connect(activities,
            &KActivities::Consumer::serviceStatusChanged,
            this,
            &ActivitySessionData::serviceStatusChanged);
}

bool ActivitySessionData::event(QEvent *event)
{
    if (event->type() == QEvent::ThreadChange) {
    qDebug() << "IN NEW THREAD ********************************************" << thread();
    qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
    //serviceStatusChanged(activities->serviceStatus());
    QTimer *t = new QTimer(this);
    t->setInterval(1000);
    t->setSingleShot(true);
    t->start();
    connect(t, SIGNAL(timeout()), this, SLOT(testSlot()));
    qDebug() << "TIMER HAS BEEN SET!";
    }

    return false;
}

void ActivitySessionData::testSlot()
{
    qDebug() << "............................. er... yes?" << thread() << activities;
    if (!activities) {
                activities = new KActivities::Controller(this);
        connect(activities,
                &KActivities::Consumer::serviceStatusChanged,
                this,
                &ActivitySessionData::serviceStatusChanged);
    }

}

ActivitySessionData::~ActivitySessionData()
{
}

void ActivitySessionData::serviceStatusChanged(KActivities::Consumer::ServiceStatus status)
{
    QTimer::singleShot(1000, this, SLOT(testSlot()));
    isEnabled = status == KActivities::Consumer::Running;
    qDebug() << "$$$$$$$$$$$$$$$$ ENABLED?????" << isEnabled;
}

ActivityRunner::ActivityRunner(QObject *parent)
    : Sprinter::Runner(parent),
      m_keywordi18n(tr("Search keyword", "activity")),
      m_keyword("activity"),
      m_defaultIcon(QIcon::fromTheme("preferences-activities"))
{
    setMatchTypesGenerated(QVector<Sprinter::QuerySession::MatchType>()
                                << Sprinter::QuerySession::ActivityType);
    setSourcesUsed(QVector<Sprinter::QuerySession::MatchSource>()
                        << Sprinter::QuerySession::FromLocalService);
}

Sprinter::RunnerSessionData *ActivityRunner::createSessionData()
{
    return new ActivitySessionData(this);
}

void ActivityRunner::match(Sprinter::RunnerSessionData *sd,
                           const Sprinter::QueryContext &context)
{
    ActivitySessionData *sessionData = qobject_cast<ActivitySessionData *>(sd);
    if (!sessionData  || !sessionData->isEnabled) {
        return;
    }

    const QString term = context.query().trimmed();
    bool list = false;
    QString name;

    if (context.isDefaultMatchesRequest()) {
        list = true;
    } else if (term.startsWith(m_keywordi18n, Qt::CaseInsensitive)) {
        if (term.size() == m_keywordi18n.size()) {
            list = true;
        } else {
            name = term.right(term.size() - m_keywordi18n.size()).trimmed();
            list = name.isEmpty();
        }
    } else if (term.startsWith(m_keyword, Qt::CaseInsensitive)) {
        if (term.size() == m_keyword.size()) {
            list = true;
        } else {
            name = term.right(term.size() - m_keyword.size()).trimmed();
            list = name.isEmpty();
        }
    } else {
        return;
    }

    QVector<Sprinter::QueryMatch> matches;
    QStringList activities = sessionData->activities->activities();
    qSort(activities);

    const QString current = sessionData->activities->currentActivity();

    if (!context.isValid()) {
        return;
    }

    if (list) {
        foreach (const QString &activity, activities) {
            if (current == activity) {
                continue;
            }

            KActivities::Info info(activity);
            addMatch(info, true, context, matches);

            if (!context.isValid()) {
                return;
            }
        }
    } else {
        foreach (const QString &activity, activities) {
            if (current == activity) {
                continue;
            }

            KActivities::Info info(activity);
            if (info.name().startsWith(name, Qt::CaseInsensitive)) {
                addMatch(info,
                         info.name().compare(name, Qt::CaseInsensitive) == 0,
                         context,
                         matches);
            }

            if (!context.isValid()) {
                return;
            }
        }
    }

    sessionData->setMatches(matches, context);
}

void ActivityRunner::addMatch(const KActivities::Info &activity, bool exact,
                              const Sprinter::QueryContext &context,
                              QVector<Sprinter::QueryMatch> &matches)
{
    Sprinter::QueryMatch match(this);
    match.setData(activity.id());
    match.setType(Sprinter::QuerySession::ActivityType);
    match.setSource(Sprinter::QuerySession::FromLocalService);
    match.setImage(image(activity, context));
    match.setText(tr("Switch to activity \"%1\"").arg(activity.name()));
    if (exact &&
        (activity.state() == KActivities::Info::Running ||
         activity.state() == KActivities::Info::Starting)) {
        match.setPrecision(Sprinter::QuerySession::ExactMatch);
    } else {
        match.setPrecision(Sprinter::QuerySession::CloseMatch);
    }
    matches << match;
}

QImage ActivityRunner::image(const KActivities::Info &activity,
                             const Sprinter::QueryContext &context)
{
    if (activity.icon().isEmpty()) {
        if (m_defaultImage.size() != context.imageSize()) {
            m_defaultImage = m_defaultIcon.pixmap(context.imageSize()).toImage();
        }

        return m_defaultImage;
    }

    QIcon icon;
    if (activity.icon().startsWith('/')) {
        icon = QIcon(activity.icon());
    } else {
        icon = QIcon::fromTheme(activity.icon());
    }

    return icon.pixmap(context.imageSize()).toImage();
}

bool ActivityRunner::exec(const Sprinter::QueryMatch &match)
{
    KActivities::Controller activities;
    QFuture<bool> rv = activities.setCurrentActivity(match.data().toString());
    rv.waitForFinished();
    return rv.result();
}

#include "moc_activities.cpp"
