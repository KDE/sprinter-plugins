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

#include "c.h"

#include <QDebug>

RunnerCSessionData::RunnerCSessionData(AbstractRunner *runner)
    : RunnerSessionData(runner)
{
}

RunnerC::RunnerC(QObject *parent)
    : AbstractRunner(parent)
{
}

RunnerSessionData *RunnerC::createSessionData()
{
    RunnerCSessionData *session = new RunnerCSessionData(this);
    session->data = "Testing";
    return session;
}

void RunnerC::match(RunnerSessionData *sessionData, const RunnerContext &context)
{
    RunnerCSessionData *session = dynamic_cast<RunnerCSessionData *>(sessionData);
    if (context.query() == "plasma") {
        QVector<QueryMatch> matches;
        QueryMatch match(this);
        match.setTitle("Plasma");
        match.setText("Rocks");
        match.setPrecision(QueryMatch::ExactMatch);
        match.setType(QueryMatch::InformationalType);
        matches << match;
        sessionData->setMatches(matches, context);
    }
}

#include "moc_c.cpp"