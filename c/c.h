#ifndef RUNNER_C
#define RUNNER_C


#include "abstractrunner.h"

class RunnerCSessionData : public RunnerSessionData
{
public:
    QString data;
};

class RunnerC : public AbstractRunner
{
    Q_OBJECT

public:
    RunnerC(QObject *parent = 0);

    RunnerSessionData *createSessionData();
    RunnableMatch *createMatcher(RunnerSessionData *sessionData, RunnerContext &context);
};

class RunnerCMatcher : public RunnableMatch
{
public:
    RunnerCMatcher(RunnerC *runner, RunnerSessionData *sessionData, const RunnerContext &context);
    void match();

private:
    RunnerC *m_runner;
};

#endif

