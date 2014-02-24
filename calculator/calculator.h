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

/**
 * This class evaluates the basic expressions given in the interface.
 */
class CalculatorRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.calculator" FILE "calculator.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    CalculatorRunner(QObject* parent = 0);
    ~CalculatorRunner();

    void match(Sprinter::RunnerSessionData *sessionData,
               const Sprinter::QueryContext &context);

private:
    QImage image(const Sprinter::QueryContext &context);

    QImage m_image;
    QIcon m_icon;
    QalculateEngine m_engine;
};

#endif
