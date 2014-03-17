/*
 * Copyright 2014  Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#ifndef FILESYSTEMRUNNER_H
#define FILESYSTEMRUNNER_H

#include <QFileInfo>
#include <QIcon>
#include <QMimeDatabase>

#include <Sprinter/Runner>

/*
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
};*/

class FilesystemRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.filesystem" FILE "filesystem.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    FilesystemRunner(QObject *parent = 0);
    ~FilesystemRunner();

    void match(Sprinter::MatchData &matchData);
    bool exec(const Sprinter::QueryMatch &match);

private:
    void createDirectoryMatch(const QFileInfo &info, Sprinter::MatchData &matchData, bool completion);
    void createFileMatch(const QFileInfo &info, Sprinter::MatchData &matchData, bool completion);
    QIcon m_directoryIcon;
    QMimeDatabase m_mimetypes;
};

#endif