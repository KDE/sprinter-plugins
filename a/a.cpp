
#include "a.h"
#include <unistd.h>

#include <QDebug>

RunnerA::RunnerA(QObject *parent)
    : AbstractRunner(parent)
{
    sleep(2);

}

#include "a.moc"


