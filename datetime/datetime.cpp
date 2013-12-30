/*
 * Copyright (C) 2013 Aaron Seigo <aseigo@kde.org>
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

static const QString dateWord = QObject::tr("date");
static const QString timeWord = QObject::tr("time");

DateTimeRunnerSessionData::DateTimeRunnerSessionData(AbstractRunner *runner)
    : RunnerSessionData(runner)
{
}

DateTimeRunner::DateTimeRunner(QObject *parent)
    : AbstractRunner(parent)
{
}

RunnerSessionData *DateTimeRunner::createSessionData()
{
    //populateTzList();
    return new DateTimeRunnerSessionData(this);
}

void DateTimeRunner::addMatch(const QString &title, const QString &clipboardText, RunnerSessionData *sessionData)
{
    QueryMatch match(this);
    match.setTitle(title);
//     match.setData(clipboardText);
    match.setPrecision(QueryMatch::ExactMatch);
    match.setType(QueryMatch::InformationalType);
//     match.setIcon(KIcon(QLatin1String( "clock" )));

    QVector<QueryMatch> matches;
    matches << match;
    sessionData->addMatches(matches);
}

void DateTimeRunner::populateTzList()
{
    QDateTime dt(QDateTime::currentDateTime());
    QString abbrev;
//     qDebug() << "POPULATING!";
    foreach (const QByteArray &tzId, QTimeZone::availableTimeZoneIds()) {
        qDebug() << tzId;
        m_tzList.insert(tzId, tzId);
        QTimeZone tz(tzId);

        abbrev = tz.abbreviation(dt);
                qDebug() << abbrev;
        if (!abbrev.isEmpty()) {
            m_tzList.insert(abbrev, abbrev.toLatin1());
        }
    }
}

QDateTime DateTimeRunner::datetime(const QString &term, bool date, QString &tzName)
{
    const QString tz = term.right(term.length() - (date ? dateWord.length() : timeWord.length()) - 1);

    if (tz.length() < 3) {
        return QDateTime();
    }

    if (tz.compare(QLatin1String("UTC"), Qt::CaseInsensitive) == 0) {
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
            tzName = it.value();
            QTimeZone tz(it.value());
            dt = QDateTime::currentDateTime();
            dt.setTimeZone(tz);
            break;
        } else if (!dt.isValid() &&
                   it.key().contains(tz, Qt::CaseInsensitive)) {
            tzName = it.value();
            QTimeZone tz(it.value());
            dt = QDateTime::currentDateTime();
            dt.setTimeZone(tz);
        }
    }

    return dt;
}

void DateTimeRunner::match(RunnerSessionData *sessionData, RunnerContext &context)
{
    DateTimeRunnerSessionData *sd = dynamic_cast<DateTimeRunnerSessionData *>(sessionData);
    if (!sd) {
        return;
    }

    const QString term = context.query();

//     qDebug() << "checking" << term;
    if (term.compare(dateWord, Qt::CaseInsensitive) == 0) {
        const QString date = QDateTime::currentDateTime().toString(Qt::SystemLocaleShortDate);
        addMatch(date, date, sd);
    } else if (term.startsWith(dateWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QDateTime dt = datetime(term, true, tzName);
        if (dt.isValid()) {
            const QString date = dt.date().toString(Qt::SystemLocaleShortDate);
            addMatch(QString("%2 (%1)").arg(tzName, date), date, sd);
        }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
        const QString time = QTime::currentTime().toString(Qt::SystemLocaleShortDate);
        addMatch(time, time, sd);
    } else if (term.startsWith(timeWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
        QString tzName;
        QDateTime dt = datetime(term, false, tzName);
        if (dt.isValid()) {
            const QString time = dt.time().toString(Qt::SystemLocaleShortDate);
            addMatch(QString("%2 (%1)").arg(tzName, time), time, sd);
        }
    }
}

#include "moc_datetime.cpp"