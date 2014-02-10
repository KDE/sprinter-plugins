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

#ifndef RUNNER_YOUTUBE
#define RUNNER_YOUTUBE

#include "sprinter/abstractrunner.h"

#include <QHash>

class QNetworkAccessManager;
class QNetworkReply;

class YoutubeSessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    YoutubeSessionData(Sprinter::AbstractRunner *runner);
    ~YoutubeSessionData();

public Q_SLOTS:
    void startQuery(const QString &query, const Sprinter::QueryContext &context);
    void queryFinished();
    void thumbRecv();

private:
    QNetworkAccessManager *m_network;
    QNetworkReply *m_reply;
    Sprinter::QueryContext m_context;
    RunnerSessionData::Busy *m_busyToken;
    QHash<QUrl, Sprinter::QueryMatch> m_thumbJobs;
};

class YoutubeRunner : public Sprinter::AbstractRunner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.youtube" FILE "youtube.json")

public:
    YoutubeRunner(QObject *parent = 0);
    Sprinter::RunnerSessionData *createSessionData();
    void match(Sprinter::RunnerSessionData *sessionData, const Sprinter::QueryContext &context);
    bool exec(const Sprinter::QueryMatch &match);

Q_SIGNALS:
    void startQuery(const QString &query, const Sprinter::QueryContext &context);
};

#endif

