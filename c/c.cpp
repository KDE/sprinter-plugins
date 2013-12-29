
#include "c.h"

#include <QDebug>

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

void RunnerC::match(RunnerSessionData *sessionData, RunnerContext &context)
{
    RunnerCSessionData *session = dynamic_cast<RunnerCSessionData *>(sessionData);
    qDebug() << "Matching ... " << context.query();
    if (context.query() == "plasma") {
        qDebug() << "Session data: " << (session ? session->data : "----") << "; query: " << context.query();
        QList<QueryMatch> matches;
        //FIXME: if the runner is deleted?
        QueryMatch match(this);
        match.setTitle("Plasma");
        match.setText("Sucks");
        match.setPrecision(QueryMatch::ExactMatch);
        match.setType(QueryMatch::InformationalType);
        matches << match;
        context.addMatches(matches);
    }
}

#include "c.moc"


