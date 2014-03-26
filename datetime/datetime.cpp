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

#include "datetime.h"

#include <QDebug>
#include <QLocale>
#include <QTimer>
#include <QTimeZone>

#include <KI18n/KLocalizedString>

static const QString dateWord = i18n("date");
static const QString timeWord = i18n("time");

DateTimeSessionData::DateTimeSessionData(Sprinter::Runner *runner)
    : Sprinter::RunnerSessionData(runner),
      m_updateTimer(new QTimer(this))
{
    m_updateTimer->setInterval(1000);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(performUpdate()));
}

void DateTimeSessionData::startUpdating()
{
    QMetaObject::invokeMethod(m_updateTimer, "start");
}

void DateTimeSessionData::stopUpdating()
{
    QMetaObject::invokeMethod(m_updateTimer, "stop");
}

bool DateTimeSessionData::shouldStartMatch(const Sprinter::QueryContext &context) const
{
    bool should = RunnerSessionData::shouldStartMatch(context);
    if (!should) {
        QMetaObject::invokeMethod(m_updateTimer, "stop");
    }
    return should;
}

void DateTimeSessionData::performUpdate()
{
    DateTimeRunner *dtr = qobject_cast<DateTimeRunner *>(runner());
    if (!dtr) {
        return;
    }

    QVector<Sprinter::QueryMatch> updates;
    Sprinter::QueryMatch update;
    foreach (const Sprinter::QueryMatch &match, matches(SynchronizedMatches)) {
        update = dtr->performMatch(match.data().toString());

        if (!update.data().isNull()) {
            update.setImage(dtr->image());
            updates << update;
        }
    }

    if (updates.isEmpty()) {
        QMetaObject::invokeMethod(m_updateTimer, "stop");
    } else {
        updateMatches(updates);
    }
}

DateTimeRunner::DateTimeRunner(QObject *parent)
    : Sprinter::Runner(parent),
      m_icon(QIcon::fromTheme("clock"))
{
}

Sprinter::QueryMatch DateTimeRunner::createMatch(const QString &title, const QString &userData, const QString &data)
{
    Sprinter::QueryMatch match;
    match.setTitle(title);
    match.setUserData(userData);
    match.setData(data);
    match.setPrecision(Sprinter::QuerySession::ExactMatch);
    match.setType(Sprinter::QuerySession::DateTimeType);
    match.setSource(Sprinter::QuerySession::FromLocalService);
//     qDebug() << "Errr... " << match.title();
    return match;
}

Sprinter::RunnerSessionData *DateTimeRunner::createSessionData()
{
    return new DateTimeSessionData(this);
}

void DateTimeRunner::populateTzList()
{
    QDateTime dt(QDateTime::currentDateTime());
    QString abbrev;
//     qDebug() << "POPULATING!";
    foreach (const QByteArray &tzId, QTimeZone::availableTimeZoneIds()) {
        qDebug() << tzId;
        QString searchableTz(tzId);
        m_tzList.insert(searchableTz.replace('_', ' '), tzId);
        QTimeZone tz(tzId);

        abbrev = tz.abbreviation(dt);
                qDebug() << abbrev;
        if (!abbrev.isEmpty()) {
            m_tzList.insert(abbrev, abbrev.toLatin1());
        }
    }
}

QDateTime DateTimeRunner::datetime(const QString &term, bool date, QString &tzName, QString &matchData)
{
    const QString tz = term.right(term.length() - (date ? dateWord.length() : timeWord.length()) - 1);

    if (tz.length() < 3) {
        return QDateTime();
    }

    if (tz.compare(QLatin1String("UTC"), Qt::CaseInsensitive) == 0) {
        matchData = (date ? dateWord : timeWord) + " UTC";
        tzName = QLatin1String("UTC");
        QDateTime UTC(QDateTime::currentDateTime());
        UTC.setTimeSpec(Qt::UTC);
        return UTC;
    }

    if (m_tzList.isEmpty()) {
        populateTzList();
    }

    QDateTime dt;
    QHashIterator<QString, QByteArray> it(m_tzList);
    while (it.hasNext()) {
        it.next();
        if (it.key().compare(tz, Qt::CaseInsensitive) == 0) {
            matchData = (date ? dateWord : timeWord) + ' ' + it.key();
            tzName = it.value();
            QTimeZone tz(it.value());
            dt = QDateTime::currentDateTime();
            dt.setTimeZone(tz);
            break;
        } else if (!dt.isValid() &&
                   it.key().contains(tz, Qt::CaseInsensitive)) {
            matchData = (date ? dateWord : timeWord) + ' ' + it.key();
            tzName = it.value();
            QTimeZone tz(it.value());
            dt = QDateTime::currentDateTime();
            dt.setTimeZone(tz);
        }
    }

    return dt;
}

void DateTimeRunner::match(Sprinter::MatchData &matchData)
{
    DateTimeSessionData *sessionData = qobject_cast<DateTimeSessionData *>(matchData.sessionData());
    if (!sessionData) {
        return;
    }

    bool isTime = false;
    Sprinter::QueryMatch match =
        performMatch(matchData.queryContext().isDefaultMatchesRequest() ?
                     timeWord : matchData.queryContext().query(), &isTime);

//     qDebug() << "got" << match.text() << (!match.data().isNull());
    if (!match.data().isNull()) {
        m_imageSize = matchData.queryContext().imageSize();
        match.setImage(image());
        matchData << match;
        if (isTime) {
            sessionData->startUpdating();
        } else {
            sessionData->stopUpdating();
        }
    } else {
        sessionData->stopUpdating();
    }
}

Sprinter::QueryMatch DateTimeRunner::performMatch(const QString &term, bool *isTime)
{
    //qDebug() << "checking" << term;
    if (term.compare(dateWord, Qt::CaseInsensitive) == 0) {
        const QString date = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
        return createMatch(date, date, dateWord);
    } else if (term.startsWith(dateWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QString matchData;
        QDateTime dt = datetime(term, true, tzName, matchData);
        if (dt.isValid()) {
            const QString date = dt.date().toString(Qt::SystemLocaleShortDate);
            return createMatch(QString("%2 (%1)").arg(tzName, date), date, matchData);
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        if (isTime) {
            *isTime = true;
        }
        const QString time = QTime::currentTime().toString(Qt::SystemLocaleLongDate);
        return createMatch(time, time, timeWord);
    } else if (term.startsWith(timeWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QString matchData;
        QDateTime dt = datetime(term, false, tzName, matchData);
        if (dt.isValid()) {
            if (isTime) {
                *isTime = true;
            }
            const QString time = dt.time().toString(Qt::SystemLocaleLongDate);
            return createMatch(QString("%2 (%1)").arg(tzName, time), time, matchData);
        }
    }

    return Sprinter::QueryMatch();
}

QImage DateTimeRunner::image()
{
    if (m_imageSize.isNull()) {
        return QImage();
    }

    if (m_image.size() != m_imageSize) {
        m_image = m_icon.pixmap(m_imageSize).toImage();
    }

    return m_image;
}

#include "moc_datetime.cpp"
