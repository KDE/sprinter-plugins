
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