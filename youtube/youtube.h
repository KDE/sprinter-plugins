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

#include "abstractrunner.h"

#include <QHash>

class QNetworkAccessManager;
class QNetworkReply;

class YoutubeSessionData : public RunnerSessionData
{
    Q_OBJECT

public:
    YoutubeSessionData(AbstractRunner *runner);
    ~YoutubeSessionData();

public Q_SLOTS:
    void startQuery(const QString &query, const QueryContext &context);
    void queryFinished();

private:
    QNetworkAccessManager *m_network;
    QNetworkReply *m_reply;
    QueryContext m_context;
    RunnerSessionData::Busy *m_busyToken;
};

class YoutubeRunner : public AbstractRunner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.youtube" FILE "youtube.json")

public:
    YoutubeRunner(QObject *parent = 0);
    RunnerSessionData *createSessionData();
    void match(RunnerSessionData *sessionData, const QueryContext &context);
    bool exec(const QueryMatch &match);

Q_SIGNALS:
    void startQuery(const QString &query, const QueryContext &context);
};

#endif

