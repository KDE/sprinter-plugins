
#include "d.h"

#include <QDebug>

RunnerD::RunnerD(QObject *parent)
    : AbstractRunner(parent)
{

}

RunnerSessionData *RunnerD::createSessionData()
{
    return 0;
}

RunnableMatch *RunnerD::createMatcher(RunnerSessionData *sessionData)
{
    return 0;
}

#include "d.moc"


