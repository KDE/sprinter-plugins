/*
 *   Copyright (C) 2011-2014 Aaron Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef ACTIVITYRUNNER_H
#define ACTIVITYRUNNER_H

#include "sprinter/querysession.h"
#include "Sprinter/Runner"

#include <KActivities/Controller>

#include <QIcon>

// this class works around threading brokenness currently
// found in KDBusConnectionPool
class KActivitiesProxy : public QObject
{
    Q_OBJECT
public:
    KActivitiesProxy();

public Q_SLOTS:
    void start();
    QFuture<bool> setCurrentActivity(const QString &activityId);

Q_SIGNALS:
    void serviceStatusChanged(KActivities::Consumer::ServiceStatus);
    void activitiesChanged(const QStringList &activities);
    void currentActivityChanged(const QString &id);

private:
    KActivities::Controller *m_activities;
};

class ActivitySessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    ActivitySessionData(Sprinter::Runner *runner);
    ~ActivitySessionData();

    QStringList activities;
    QString currentActivity;
    bool isEnabled;

public Q_SLOTS:
    void serviceStatusChanged(KActivities::Consumer::ServiceStatus);
    void activitiesChanged(const QStringList &activities);
    void currentActivityChanged(const QString &id);
    bool setCurrentActivity(const QString &activityId);

private:
    KActivitiesProxy *m_activitiesProxy;
};

class ActivityRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.activities" FILE "activities.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    ActivityRunner(QObject *parent = 0);

    Sprinter::RunnerSessionData *createSessionData();

    void match(Sprinter::MatchData &matchData);
    bool exec(const Sprinter::QueryMatch &match);

private:
    void addMatch(const KActivities::Info &activity,
                  Sprinter::QuerySession::MatchPrecision precision,
                  Sprinter::MatchData &matchData);
    QImage image(const KActivities::Info &activity,
                 const Sprinter::QueryContext &context);

    const QString m_keywordi18n;
    const QString m_keyword;
    QIcon m_defaultIcon;
    QImage m_defaultImage;
};

#endif
