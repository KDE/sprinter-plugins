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

#ifndef RUNNER_DATETIME
#define RUNNER_DATETIME

#include "abstractrunner.h"

#include <QTimeZone>

class DateTimeRunner : public AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent = 0);
    void match(RunnerSessionData *sessionData, const RunnerContext &context);

private:
    QDateTime datetime(const QString &term, bool date, QString &tzName);
    QueryMatch createMatch(const QString &title, const QString &clipboard, RunnerSessionData *sessionData, const RunnerContext &context);
    void populateTzList();

    QHash<QString, QByteArray> m_tzList;
};

#endif

