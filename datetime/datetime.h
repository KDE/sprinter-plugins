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

private:
    QDateTime datetime(const QString &term, bool date, QString &tzName);
    void addMatch(const QString &title, const QString &clipboard, RunnerSessionData *sessionData);
    void populateTzList();

    QHash<QString, QByteArray> m_tzList;
};

#endif

