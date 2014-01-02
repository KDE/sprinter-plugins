 
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

#include "youtube.h"

#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

static const QString shortTrigger = QObject::tr("yt ");
static const QString longTrigger = QObject::tr("video ");
// three vars are page size, offset and query (1, 2, 3 resp)
static const QString url = "http://gdata.youtube.com/feeds/api/videos?max-results=%1&starts-index=%2&alt=json&q=%3";

#include <QThread>
YoutubeSessionData::YoutubeSessionData(AbstractRunner *runner)
    : RunnerSessionData(runner),
      m_network(new QNetworkAccessManager(this)),
      m_reply(0)
{
    connect(runner, SIGNAL(startQuery(QString,RunnerContext)),
            this, SLOT(startQuery(QString, RunnerContext)));
}

void YoutubeSessionData::startQuery(const QString &query, const RunnerContext &context)
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = 0;
    }

    m_context = context;
    if (!m_context.isValid()) {
        qDebug() << "REJECTING YOUTUBE REPLY: context is already invalid";
        return;
    }

    QNetworkRequest request(url.arg(QString::number(resultsPageSize()),
                                    QString::number(resultsOffset()),
                                    query));
    QNetworkAccessManager *network = new QNetworkAccessManager(this);
    QNetworkReply *reply = network->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(queryFinished()));

    m_reply = reply;
}

void YoutubeSessionData::queryFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if (!m_reply || reply != m_reply) {
        qDebug() << "query finished .. but no reply" << sender();
        return;
    }

    if (m_context.isValid()) {
        qDebug() << "We have our reply!";
        //setMatches(matches, context);
    }

    m_reply->deleteLater();
    m_reply = 0;
}


YoutubeRunner::YoutubeRunner(QObject *parent)
    : AbstractRunner(parent)
{
    setAsync(true);
}

RunnerSessionData *YoutubeRunner::createSessionData()
{
    return new YoutubeSessionData(this);
}

void YoutubeRunner::match(RunnerSessionData *sessionData, const RunnerContext &context)
{
    const QString term = context.query();
    QString query;
    if (term.startsWith(shortTrigger, Qt::CaseInsensitive)) {
        query = term.right(term.length() - shortTrigger.length());
    } else if (term.startsWith(longTrigger, Qt::CaseInsensitive)) {
        query = term.right(term.length() - longTrigger.length());
    } else {
        return;
    }

    if (query.size() < 3) {
        return;
    }

    qDebug() <<" should be matching... " << query;
    YoutubeSessionData *sd = dynamic_cast<YoutubeSessionData *>(sessionData);
    if (!sd) {
        return;
    }

    emit startQuery(query, context);
}

#include "moc_youtube.cpp"