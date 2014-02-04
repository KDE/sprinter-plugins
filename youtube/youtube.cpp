 
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
#include <QDesktopServices>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QTime>

#include <querycontext.h>

static const QString shortTrigger = QObject::tr("yt ");
static const QString longTrigger = QObject::tr("video ");
// three vars are page size, offset and query (1, 2, 3, 4 resp)
static const QString url = "http://gdata.youtube.com/feeds/api/videos?max-results=%1&start-index=%2&alt=json&q=%4";

YoutubeSessionData::YoutubeSessionData(Sprinter::AbstractRunner *runner)
    : Sprinter::RunnerSessionData(runner),
      m_network(new QNetworkAccessManager(this)),
      m_reply(0),
      m_busyToken(0)
{
    connect(runner, SIGNAL(startQuery(QString,Sprinter::QueryContext)),
            this, SLOT(startQuery(QString, Sprinter::QueryContext)));
}

YoutubeSessionData::~YoutubeSessionData()
{
    delete m_busyToken;
}

void YoutubeSessionData::startQuery(const QString &query, const Sprinter::QueryContext &context)
{
    if (m_reply) {
        m_reply->deleteLater();
        m_reply = 0;
    }

    m_context = context;
    if (!m_context.isValid()) {
        qDebug() << "REJECTING YOUTUBE REPLY: context is already invalid";
        delete m_busyToken;
        m_busyToken = 0;
        return;
    } else if (!m_busyToken) {
        m_busyToken = new Sprinter::RunnerSessionData::Busy(this);
    }

    QNetworkRequest request(url.arg(QString::number(resultsPageSize()),
                                    QString::number(resultsOffset() + 1),
                                    query));
    QNetworkAccessManager *network = new QNetworkAccessManager(this);
    QNetworkReply *reply = network->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(queryFinished()));

    m_reply = reply;
}

void YoutubeSessionData::queryFinished()
{
    QNetworkReply *reply = qobject_cast<QNetworkReply *>(sender());

    if (!m_reply) {
        qDebug() << "query finished .. but no reply" << sender();
        return;
    }

    if (reply != m_reply) {
       reply->deleteLater();
       qDebug() << "Late response";
       return;
    }

    delete m_busyToken;
    m_busyToken = 0;

    if (m_context.isValid()) {
        bool ok = false;
        QByteArray data = reply->readAll();
        QJsonParseError *error = 0;
        QJsonDocument doc = QJsonDocument::fromJson(data, error);
        qDebug() << "We have our reply!" << reply->url() << ok;
        if (!error) {
            QJsonObject obj = doc.object();

            QVector<Sprinter::QueryMatch> matches;
            const QJsonArray entries = obj["feed"].toObject()["entry"].toArray();
            QTime t;
            for (QJsonArray::const_iterator it = entries.begin();
                 it != entries.end();
                 ++it) {
                const QJsonObject entry = (*it).toObject();
                if (entry.isEmpty()) {
                    continue;
                }

                const QJsonObject media = entry["media$group"].toObject();
                const QString title = media["media$title"].toObject()["$t"].toString();
                int seconds = media["yt$duration"].toObject()["seconds"].toString().toInt();
                const QString author = entry["author"].toArray().first().toObject()["name"].toObject()["$t"].toString();
                const QString desc = entry["content"].toObject()["$t"].toString();
                const QString url = entry["link"].toArray().first().toObject()["href"].toString();
//                 qDebug() << "================================";
//                 qDebug() << title << seconds << time << desc;

                QString time;
                if (seconds < 60) {
                    time = "00:" + QString(seconds > 9 ? "" : "0") + QString::number(seconds);
                } else if (seconds < 60*60) {
                    int minutes = seconds / 60;
                    seconds = seconds % 60;
                    time = QString::number(minutes) + ":" +
                           (seconds > 9 ? "" : "0") + QString::number(seconds);
                } else {
                    int minutes = seconds / 60;
                    int hours = minutes / 60;
                    minutes = minutes % 60;
                    seconds = seconds % 60;
                    time = QString::number(hours) + ":" +
                           (minutes > 9 ? "" : "0") + QString::number(minutes) + ":" +
                           (seconds > 9 ? "" : "0") + QString::number(seconds);
                }

                Sprinter::QueryMatch match(runner());
                match.setTitle(tr("%1 (%2, %3)").arg(title, author, time));
                match.setText(desc);
                match.setType(Sprinter::QuerySession::VideoType);
                match.setSource(Sprinter::QuerySession::FromNetworkService);
                match.setPrecision(Sprinter::QuerySession::CloseMatch);
                match.setUserData(url);
                match.setData(url);
                matches << match;
            }
//             qDebug() <<" **********" << matches.count();
            setMatches(matches, m_context);
            setCanFetchMoreMatches(true, m_context);
        }
    }

    reply->deleteLater();
    m_reply = 0;
}


YoutubeRunner::YoutubeRunner(QObject *parent)
    : AbstractRunner(parent)
{
    setMatchTypesGenerated(QVector<Sprinter::QuerySession::MatchType>()
                                << Sprinter::QuerySession::VideoType);
    setSourcesUsed(QVector<Sprinter::QuerySession::MatchSource>()
                        << Sprinter::QuerySession::FromNetworkService);
}

Sprinter::RunnerSessionData *YoutubeRunner::createSessionData()
{
    return new YoutubeSessionData(this);
}

void YoutubeRunner::match(Sprinter::RunnerSessionData *sessionData, const Sprinter::QueryContext &context)
{
    const QString term = context.query();
    QString query;
    if (term.startsWith(shortTrigger, Qt::CaseInsensitive)) {
        query = term.right(term.length() - shortTrigger.length());
    } else if (term.startsWith(longTrigger, Qt::CaseInsensitive)) {
        query = term.right(term.length() - longTrigger.length());
    } else {
        sessionData->setMatches(QVector<Sprinter::QueryMatch>(), context);
        return;
    }

    if (query.size() < 3) {
        sessionData->setMatches(QVector<Sprinter::QueryMatch>(), context);
        return;
    }

    qDebug() <<" should be matching... " << query;
    YoutubeSessionData *sd = dynamic_cast<YoutubeSessionData *>(sessionData);
    if (!sd) {
        sessionData->setMatches(QVector<Sprinter::QueryMatch>(), context);
        return;
    }

    emit startQuery(query, context);
}

bool YoutubeRunner::exec(const Sprinter::QueryMatch &match)
{
    return QDesktopServices::openUrl(match.data().toUrl());
}

#include "moc_youtube.cpp"