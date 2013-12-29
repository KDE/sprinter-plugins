
#include "c.h"

#include <QDebug>

RunnerCSessionData::RunnerCSessionData(AbstractRunner *runner)
    : RunnerSessionData(runner)
{
}

RunnerC::RunnerC(QObject *parent)
    : AbstractRunner(parent)
{
}

RunnerSessionData *RunnerC::createSessionData()
{
    RunnerCSessionData *session = new RunnerCSessionData(this);
    session->data = "Testing";
    return session;
}

void RunnerC::match(RunnerSessionData *sessionData, RunnerContext &context)
{
    RunnerCSessionData *session = dynamic_cast<RunnerCSessionData *>(sessionData);
    qDebug() << "Matching ... " << context.query();
    if (context.query() == "plasma") {
        qDebug() << "Session data: " << (session ? session->data : "----") << "; query: " << context.query();
        QVector<QueryMatch> matches;
        QueryMatch match(this);
        match.setTitle("Plasma");
        match.setText("Sucks");
        match.setPrecision(QueryMatch::ExactMatch);
        match.setType(QueryMatch::InformationalType);
        matches << match;
        sessionData->addMatches(matches);
    }
}

#include "c.moc"


