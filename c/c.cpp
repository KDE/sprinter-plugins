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

#include <unistd.h>

#include <QDebug>

RunnerC::RunnerC(QObject *parent)
    : Sprinter::Runner(parent),
      m_icon(QIcon::fromTheme("kate"))
{
    setGeneratesDefaultMatches(true);
}

void RunnerC::match(Sprinter::RunnerSessionData *sessionData, const Sprinter::QueryContext &context)
{
    QVector<Sprinter::QueryMatch> matches;
    if (context.isDefaultMatchesRequest() || context.query() == "plasma") {
        Sprinter::QueryMatch match(this);
        match.setTitle("Plasma");
        match.setText("Rocks");
        match.setPrecision(Sprinter::QuerySession::ExactMatch);
        match.setType(Sprinter::QuerySession::DesktopType);
        match.setSource(Sprinter::QuerySession::FromDesktopShell);
        match.setData("time");
        match.setIsSearchTerm(true);
        match.setImage(m_icon.pixmap(context.imageSize()).toImage());
        matches << match;
    }

    sessionData->setMatches(matches, context);
}

bool RunnerC::exec(const Sprinter::QueryMatch &match)
{
    qDebug() << "********* EXEC ****************";
    sleep(1);
    return true;
}

#include "moc_c.cpp"