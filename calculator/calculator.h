/*
 *   Copyright 2007 Barış Metin <baris@pardus.org.tr>
 *   Copyright 2010 Matteo Agostinelli <agostinelli@gmail.com>
 *   Copyright 2014 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef CALCULATORRUNNER_H
#define CALCULATORRUNNER_H

#include <QIcon>

#include <sprinter/runner.h>

#include "qalculate_engine.h"

class CalculatorSessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    CalculatorSessionData(Sprinter::Runner *runner)
        : RunnerSessionData(runner),
          m_engine(new QalculateEngine(this))
    {
    }

    QalculateEngine *m_engine;
};

class CalculatorRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.calculator" FILE "calculator.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    CalculatorRunner(QObject* parent = 0);
    ~CalculatorRunner();

    Sprinter::RunnerSessionData *createSessionData();
    void match(Sprinter::MatchData &matchData);

private:
    QIcon m_icon;
};

#endif
