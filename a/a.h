#ifndef RUNNER_A
#define RUNNER_A


#include "abstractrunner.h"

class RunnerA : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerA(QObject *parent = 0);

    RunnerSessionData *createSessionData();
    RunnableMatch *createMatcher(RunnerSessionData *sessionData);
};

#endif

