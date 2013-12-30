
#include "a.h"
#include <unistd.h>

#include <QDebug>

RunnerA::RunnerA(QObject *parent)
    : AbstractRunner(parent)
{
    //sleep(2);
}

#include "moc_a.cpp"