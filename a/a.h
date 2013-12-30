#ifndef RUNNER_A
#define RUNNER_A

#include "abstractrunner.h"

#include <QTimeZone>

class DateTimeRunnerSessionData : public RunnerSessionData
{
public:
    DateTimeRunnerSessionData(AbstractRunner *runner);
    QTimeZone m_tz;
};

class DateTimeRunner : public AbstractRunner
{
    Q_OBJECT

public:
    DateTimeRunner(QObject *parent = 0);
    RunnerSessionData *createSessionData();
    void match(RunnerSessionData *sessionData, RunnerContext &context);
};

#endif

