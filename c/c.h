/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
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

#ifndef RUNNER_C
#define RUNNER_C

#include "abstractrunner.h"

class RunnerC : public Sprinter::AbstractRunner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.c" FILE "c.json")

public:
    RunnerC(QObject *parent = 0);

    void match(Sprinter::RunnerSessionData *sessionData, const Sprinter::QueryContext &context);
    bool exec(const Sprinter::QueryMatch &match);
};

#endif

