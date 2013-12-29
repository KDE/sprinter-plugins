
#include "c.h"

#include <QDebug>

RunnerCMatcher::RunnerCMatcher(RunnerC *runner, RunnerSessionData *sessionData)
    : RunnableMatch(sessionData)
{
}

void RunnerCMatcher::match()
{
    RunnerCSessionData *session = static_cast<RunnerCSessionData *>(sessionData());

    if (context().query() == "plasma") {
        qDebug() << "Session data: " << (session ? session->data : "----") << "; query: " << context().query();
        QList<QueryMatch> matches;
        //FIXME: if the runner is deleted?
        QueryMatch match(m_runner);
        match.setTitle("Plasma");
        match.setText("Sucks");
        match.setPrecision(QueryMatch::ExactMatch);
        match.setType(QueryMatch::InformationalType);
        matches << match;
        context().addMatches(matches);
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

RunnableMatch *RunnerC::createMatcher(RunnerSessionData *sessionData)
{
    RunnerCMatcher *matcher = new RunnerCMatcher(this, sessionData);
    return matcher;
}

#include "c.moc"


