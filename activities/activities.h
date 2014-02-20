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

#include "sprinter/runner.h"

#include <KActivities/Controller>

#include <QIcon>

class ActivitySessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    ActivitySessionData(Sprinter::Runner *runner);
    ~ActivitySessionData();
    bool event(QEvent *event);

    KActivities::Controller *activities;
    bool isEnabled;

public Q_SLOTS:
    void testSlot();
    void serviceStatusChanged(KActivities::Consumer::ServiceStatus);
};

class ActivityRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.activities" FILE "activities.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    ActivityRunner(QObject *parent = 0);

    Sprinter::RunnerSessionData *createSessionData();

    void match(Sprinter::RunnerSessionData *sessionData,
               const Sprinter::QueryContext &context);
    bool exec(const Sprinter::QueryMatch &match);

private:
    void addMatch(const KActivities::Info &activity, bool exact,
                  const Sprinter::QueryContext &context,
                  QVector<Sprinter::QueryMatch> &matches);
    QImage image(const KActivities::Info &activity,
                 const Sprinter::QueryContext &context);

    const QString m_keywordi18n;
    const QString m_keyword;
    QIcon m_defaultIcon;
    QImage m_defaultImage;
};

#endif
