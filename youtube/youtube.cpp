 
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
#include <QTime>

#include <qjson/serializer.h>
static const QString shortTrigger = QObject::tr("yt ");
static const QString longTrigger = QObject::tr("video ");
// three vars are page size, offset and query (1, 2, 3, 4 resp)
static const QString url = "http://gdata.youtube.com/feeds/api/videos?max-results=%1&start-index=%2&alt=json&q=%4";

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
    }

    if (m_context.isValid()) {
        bool ok = false;
        QByteArray data = reply->readAll();
        const QVariantMap parsedJson = m_parser.parse(data, &ok).toMap();
        qDebug() << "We have our reply!" << reply->url() << ok;
        if (ok) {
            QVector<QueryMatch> matches;
//             QJson::Serializer serializer;
//             serializer.setIndentMode(QJson::IndentFull);
            QVariantList entries = parsedJson.value("feed").toMap().value("entry").toList();
            QListIterator<QVariant> it(entries);
            QTime t;
            while (it.hasNext()) {
                const QVariantMap entry = it.next().toMap();
                const QVariantMap media = entry.value("media$group").toMap();
                const QString title = media.value("media$title").toMap().value("$t").toString();
                int seconds = media.value("yt$duration").toMap().value("seconds").toInt();
                const QString author = entry.value("author").toList().first().toMap().value("name").toMap().value("$t").toString();
                const QString desc = entry.value("content").toMap().value("$t").toString();
//                 qDebug() << "================================";
//                 qDebug() << title << seconds << time << desc;
//                 qDebug() << serializer.serialize(entry);

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

                QueryMatch match(runner());
                match.setTitle(tr("%1 (%2, %3)").arg(title, author, time));
                match.setText(desc);
                matches << match;
            }
//             qDebug() <<" **********" << matches.count();
            setMatches(matches, m_context);
        }
    }

    reply->deleteLater();
    m_reply = 0;
}


YoutubeRunner::YoutubeRunner(QObject *parent)
    : AbstractRunner(parent)
{
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
        sessionData->setMatches(QVector<QueryMatch>(), context);
        return;
    }

    if (query.size() < 3) {
        sessionData->setMatches(QVector<QueryMatch>(), context);
        return;
    }

    qDebug() <<" should be matching... " << query;
    YoutubeSessionData *sd = dynamic_cast<YoutubeSessionData *>(sessionData);
    if (!sd) {
        sessionData->setMatches(QVector<QueryMatch>(), context);
        return;
    }

    emit startQuery(query, context);
}

#include "moc_youtube.cpp"