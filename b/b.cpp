
#include "b.h"

#include <QDebug>

RunnerB::RunnerB(QObject *parent)
    : AbstractRunner(parent)
{
}

RunnableMatch *RunnerB::createMatcher(RunnerSessionData *sessionData)
{
    return 0;
}

#include "b.moc"


