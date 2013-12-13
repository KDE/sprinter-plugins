
#include "a.h"
#include <unistd.h>

#include <QDebug>

RunnerA::RunnerA(QObject *parent)
    : AbstractRunner(parent)
{
    sleep(2);

}

RunnerSessionData *RunnerA::createSessionData()
{
    return 0;
}

RunnableMatch *RunnerA::createMatcher(RunnerSessionData *sessionData, RunnerContext &context)
{
    return 0;
}

#include "a.moc"


