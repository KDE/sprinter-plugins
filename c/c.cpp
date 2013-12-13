
#include "c.h"

#include <QDebug>

RunnerCMatcher::RunnerCMatcher(RunnerC *runner, RunnerSessionData *sessionData, const RunnerContext &context)
    : RunnableMatch(sessionData, context)
{
}

void RunnerCMatcher::match()
{
    RunnerCSessionData *session = static_cast<RunnerCSessionData *>(sessionData());

    if (query() == "plasma") {
        qDebug() << "Session data: " << (session ? session->data : "----") << "; query: " << query();
        QList<QueryMatch> matches;
        //FIXME: if the runner is deleted?
        QueryMatch match(m_runner);
        match.setText("Sucks");
        matches << match;
        addMatches(matches);
    }
}

RunnerC::RunnerC(QObject *parent)
    : AbstractRunner(parent)
{

}

RunnerSessionData *RunnerC::createSessionData()
{
    RunnerCSessionData *session = new RunnerCSessionData;
    session->data = "Testing";
    return session;
}

RunnableMatch *RunnerC::createMatcher(RunnerSessionData *sessionData, RunnerContext &context)
{
    RunnerCMatcher *matcher = new RunnerCMatcher(this, sessionData, context);
    return matcher;
}

#include "c.moc"


