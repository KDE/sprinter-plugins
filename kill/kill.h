/* Copyright 2009  <Jan Gerrit Marker> <jangerrit@weiler-marker.com>
 * Copyright 2014  Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public 
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef KILLRUNNER_H
#define KILLRUNNER_H

#include <QIcon>
#include <QTime>

#include <Sprinter/Runner>

namespace KSysGuard
{
    class Processes;
    class Process;
}

class KillSessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    KillSessionData(Sprinter::Runner *runner);
    ~KillSessionData();

    KSysGuard::Processes *m_processes;

public Q_SLOTS:
    void updateProcessTable();

private:
    QTime m_lastUpdate;
};

class KillRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.kill" FILE "kill.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    KillRunner(QObject *parent = 0);
    ~KillRunner();

    Sprinter::RunnerSessionData *createSessionData();

    void match(Sprinter::MatchData &matchData);
    bool exec(const Sprinter::QueryMatch &match);

private:
    /** @param uid the uid of the user
      * @return the username of the user with the UID uid
      */
    QString getUserName(qlonglong uid);

    /** The trigger word */
    const QString m_triggerWord;
    QIcon m_icon;
};

#endif