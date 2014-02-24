/*
 *   Copyright 2007 Barış Metin <baris@pardus.org.tr>
 *   Copyright 2006 David Faure <faure@kde.org>
 *   Copyright 2007 Richard Moore <rich@kde.org>
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

#include "calculator.h"

#include <QDebug>
#include <QLocale>

#include <sprinter/querymatch.h>

CalculatorRunner::CalculatorRunner(QObject* parent)
    : Sprinter::Runner(parent),
      m_icon(QIcon::fromTheme("accessories-calculator"))
{
}

CalculatorRunner::~CalculatorRunner()
{
}

class SetMatches
{
public:
    SetMatches(Sprinter::RunnerSessionData *sessionData,
               const Sprinter::QueryContext &context,
               QVector<Sprinter::QueryMatch> *matches)
        : m_sessionData(sessionData),
          m_context(context),
          m_matches(matches)
    {
    }

    ~SetMatches()
    {
        m_sessionData->setMatches(*m_matches, m_context);
    }

private:
    Sprinter::RunnerSessionData *m_sessionData;
    Sprinter::QueryContext m_context;
    QVector<Sprinter::QueryMatch> *m_matches;
};

void CalculatorRunner::match(Sprinter::RunnerSessionData *sessionData,
                             const Sprinter::QueryContext &context)
{
    const QString term = context.query();
    QString cmd = term;

    //no meanless space between friendly guys: helps simplify code
    cmd = cmd.trimmed().remove(' ');

    QVector<Sprinter::QueryMatch> matches;
    SetMatches matchSetter(sessionData, context, &matches);

    if (cmd.toLower() == "universe" || cmd.toLower() == "life") {
        Sprinter::QueryMatch match(this);
        match.setPrecision(Sprinter::QuerySession::ExactMatch);
        match.setType(Sprinter::QuerySession::MathAndUnitsType);
        match.setSource(Sprinter::QuerySession::FromInternalSource);
        match.setImage(image(context));
        match.setText("42");
        match.setData("42");
        matches << match;
        return;
    }

    bool toHex = cmd.startsWith(QLatin1String("hex="));
    bool startsWithEquals = !toHex && cmd[0] == '=';

    if (toHex || startsWithEquals) {
        cmd.remove(0, cmd.indexOf('=') + 1);
    } else if (cmd.endsWith('=')) {
        cmd.chop(1);
    } else {
        bool foundDigit = false;
        for (int i = 0; i < cmd.length(); ++i) {
            QChar c = cmd.at(i);
            if (c.isLetter()) {
                // not just numbers and symbols, so we return
                return;
            }

            if (c.isDigit()) {
                foundDigit = true;
            }
        }

        if (!foundDigit) {
            return;
        }
    }

    if (cmd.isEmpty()) {
        return;
    }

    if (cmd.contains(QLocale::system().decimalPoint(), Qt::CaseInsensitive)) {
         cmd = cmd.replace(QLocale::system().decimalPoint(), QChar('.'), Qt::CaseInsensitive);
    }

    QString result;
    try {
        result = m_engine.evaluate(term);
        result = result.replace('.', QLocale::system().decimalPoint(), Qt::CaseInsensitive);
    } catch(std::exception &e) {
        qDebug() << "qalculate error: " << e.what();
    }

    if (!result.isEmpty() && result != cmd) {
        if (toHex) {
            result = "0x" + QString::number(result.toInt(), 16).toUpper();
        }

        Sprinter::QueryMatch match(this);
        match.setPrecision(Sprinter::QuerySession::ExactMatch);
        match.setType(Sprinter::QuerySession::MathAndUnitsType);
        match.setSource(Sprinter::QuerySession::FromInternalSource);
        match.setImage(image(context));
        match.setText(result);
        match.setData(result);
        matches << match;
    }
}

QImage CalculatorRunner::image(const Sprinter::QueryContext &context)
{
    if (m_image.size() != context.imageSize()) {
        m_image = m_icon.pixmap(context.imageSize()).toImage();
    }

    return m_image;
}

#include "moc_calculator.cpp"
