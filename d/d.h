#ifndef RUNNER_D
#define RUNNER_D

#include "abstractrunner.h"

class RunnerD : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerD(QObject *parent = 0);

    RunnerSessionData *createSessionData();
    RunnableMatch *createMatcher(RunnerSessionData *sessionData, RunnerContext &context);
};

#endif

