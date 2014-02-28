/*
 *   Copyright (C) 2006-2014 Aaron Seigo <aseigo@kde.org>
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

#ifndef SERVICERUNNER_H
#define SERVICERUNNER_H


#include <KService>

#include <Sprinter/Runner>

class ApplicationsRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.applications" FILE "applications.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    ApplicationsRunner(QObject *parent = 0);
    ~ApplicationsRunner();

    void match(Sprinter::MatchData &matchData);
    bool exec(const Sprinter::QueryMatch &match);

private:
    void generateTopLevelGroups(Sprinter::MatchData &matchData,
                                const Sprinter::QueryContext &context);
    void showEntriesInGroup(const QString &relPath,
                            Sprinter::MatchData &matchData,
                            const Sprinter::QueryContext &context);
    void setupMatch(const KService::Ptr &service,
                    Sprinter::QueryMatch &action,
                    const Sprinter::QueryContext &context);
};


#endif

