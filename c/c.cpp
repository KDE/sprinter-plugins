
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
    if (context.query() == "plasma") {
        QVector<QueryMatch> matches;
        QueryMatch match(this);
        match.setTitle("Plasma");
        match.setText("Rocks");
        match.setPrecision(QueryMatch::ExactMatch);
        match.setType(QueryMatch::InformationalType);
        matches << match;
        sessionData->addMatches(matches);
    }
}

#include "moc_c.cpp"