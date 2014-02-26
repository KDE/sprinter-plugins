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

#include "sprinter/runner.h"

#include <QIcon>

class QTimer;

class DateTimeSessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    DateTimeSessionData(Sprinter::Runner *runner);
    bool shouldStartMatch(const Sprinter::QueryContext &context) const;

private Q_SLOTS:
    void performUpdate();

private:
    QTimer *m_updateTimer;
};

class DateTimeRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.datetime" FILE "datetime.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    DateTimeRunner(QObject *parent = 0);
    Sprinter::RunnerSessionData *createSessionData();
    void match(Sprinter::MatchData &matchData);
    Sprinter::QueryMatch performMatch(const QString &term);

    QImage image();

Q_SIGNALS:
    void startUpdating();
    void stopUpdating();

private:
    QDateTime datetime(const QString &term, bool date, QString &tzName, QString &matchData);
    Sprinter::QueryMatch createMatch(const QString &title, const QString &userData, const QString &data);
    void populateTzList();

    QHash<QString, QByteArray> m_tzList;
    QIcon m_icon;
    QImage m_image;
    QSize m_imageSize;
};

#endif

