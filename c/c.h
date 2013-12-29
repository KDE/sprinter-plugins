#ifndef RUNNER_C
#define RUNNER_C


#include "abstractrunner.h"

class RunnerCSessionData : public RunnerSessionData
{
public:
    RunnerCSessionData(AbstractRunner *runner);
    QString data;
};

class RunnerC : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerC(QObject *parent = 0);

    RunnerSessionData *createSessionData();
    void match(RunnerSessionData *sessionData, RunnerContext &context);
};

#endif

