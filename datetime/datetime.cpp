
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
    return new DateTimeRunnerSessionData(this);
}

void DateTimeRunner::match(RunnerSessionData *sessionData, RunnerContext &context)
{
    DateTimeRunnerSessionData *sd = dynamic_cast<DateTimeRunnerSessionData *>(sessionData);
    if (!sd) {
        return;
    }

    const QString term = context.query();
    QVector<QueryMatch> matches;

    qDebug() << "checking" << term;
    if (term.compare(dateWord, Qt::CaseInsensitive) == 0) {
        QDateTime dt = QDateTime::currentDateTime();
        QueryMatch match(this);
        match.setTitle(dt.toString(Qt::SystemLocaleShortDate));
        match.setPrecision(QueryMatch::ExactMatch);
        match.setType(QueryMatch::InformationalType);
        matches << match;
    } else if (term.startsWith(dateWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
//         QString tzName;
//         QDateTime dt = datetime(term, true, tzName);
//         if (dt.isValid()) {
//             const QString date = KGlobal::locale()->formatDate(dt.date());
//             addMatch(i18n("The date in %1 is %2", tzName, date), date, context);
//         }
    } else if (term.compare(timeWord, Qt::CaseInsensitive) == 0) {
//         const QString time = KGlobal::locale()->formatTime(QTime::currentTime());
//         addMatch(i18n("The current time is %1", time), time, context);
        QTime dt = QTime::currentTime();
        QueryMatch match(this);
        match.setTitle(dt.toString(Qt::SystemLocaleShortDate));
        match.setPrecision(QueryMatch::ExactMatch);
        match.setType(QueryMatch::InformationalType);
        matches << match;
    } else if (term.startsWith(timeWord + QLatin1Char( ' ' ), Qt::CaseInsensitive)) {
//         QString tzName;
//         QDateTime dt = datetime(term, true, tzName);
//         if (dt.isValid()) {
//             const QString time = KGlobal::locale()->formatTime(dt.time());
//             addMatch(i18n("The current time in %1 is %2", tzName, time), time, context);
//         }
    }

    sessionData->addMatches(matches);
}

#include "moc_datetime.cpp"