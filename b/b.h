#ifndef RUNNER_B
#define RUNNER_B


#include "abstractrunner.h"


class RunnerB : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerB(QObject *parent = 0);
    RunnableMatch *createMatcher(RunnerSessionData *sessionData);
};

#endif

