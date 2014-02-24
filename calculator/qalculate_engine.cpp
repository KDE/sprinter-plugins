/*
*   Copyright 2010 Matteo Agostinelli <agostinelli@gmail.com>
*   Copyright 2014 Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2 or
*   (at your option) any later version.
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

#include "qalculate_engine.h"

#include <libqalculate/Calculator.h>
#include <libqalculate/ExpressionItem.h>
#include <libqalculate/Unit.h>
#include <libqalculate/Prefix.h>
#include <libqalculate/Variable.h>
#include <libqalculate/Function.h>

#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QMutexLocker>
#include <QNetworkReply>
#include <QNetworkRequest>

QAtomicInt QalculateEngine::s_counter;

QalculateEngine::QalculateEngine(QObject* parent):
    QObject(parent),
    m_network(0),
    m_networkReply(0)
{
    QMutexLocker lock(&m_mutex);
    s_counter.ref();
    if (!CALCULATOR) {
        new Calculator();
        CALCULATOR->terminateThreads();
        CALCULATOR->loadGlobalDefinitions();
        CALCULATOR->loadLocalDefinitions();
        CALCULATOR->loadGlobalCurrencies();
        CALCULATOR->loadExchangeRates();
    }
}

QalculateEngine::~QalculateEngine()
{
    if (s_counter.deref()) {
        delete CALCULATOR;
        CALCULATOR = NULL;
    }
}

void QalculateEngine::updateExchangeRates()
{
    QMutexLocker lock(&m_mutex);
    if (m_exchangeRatesUpdated.isNull()) {
        QFileInfo info(CALCULATOR->getExchangeRatesFileName().c_str());
        if (info.exists()) {
            m_exchangeRatesUpdated = info.lastModified();
        }
    }

    if ((!m_exchangeRatesUpdated.isNull() &&
         m_exchangeRatesUpdated.daysTo(QDateTime::currentDateTime()) < 1) ||
        (!m_lastAttempt.isNull() &&
         m_lastAttempt.elapsed() < (30 * 60 * 1000)) ||
        m_networkReply) {
        // limit fetches to once per day, or one try every half hour
        qDebug() <<" NOT EVEN BOTHERING";
        return;
    }

    if (!m_network) {
        m_network = new QNetworkAccessManager(this);
    }

    m_lastAttempt.restart();
    QNetworkRequest request(QUrl("http://www.ecb.europa.eu/stats/eurofxref/eurofxref-daily.xml"));
    m_networkReply = m_network->get(request);
    connect(m_networkReply, &QNetworkReply::finished,
            this, &QalculateEngine::exchangeRatesFetched);
}

void QalculateEngine::exchangeRatesFetched()
{
    if (!m_networkReply || !CALCULATOR) {
        return;
    }

    m_exchangeRatesUpdated = QDateTime::currentDateTime();
    if (m_networkReply->error() == QNetworkReply::NoError) {
        // the exchange rates have been successfully updated, now load them
        QFile f(CALCULATOR->getExchangeRatesFileName().c_str());
        QByteArray data = m_networkReply->readAll();
        if (!data.isEmpty() &&
            f.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
            f.write(data);
            f.close();
            CALCULATOR->loadExchangeRates();
        }
    } else {
        qDebug() << "The exchange rates could not be updated. The following error has been reported:" << m_networkReply->errorString();
    }

    QMutexLocker locker(&m_mutex);
    m_network->deleteLater();
    m_network = 0;
    m_networkReply->deleteLater();
    m_networkReply = 0;
}

QString QalculateEngine::evaluate(const QString& expression)
{
    if (expression.isEmpty()) {
        return expression;
    }

    QMetaObject::invokeMethod(this, "updateExchangeRates");

    QString input = expression;
    QByteArray ba = input.replace(QChar(0xA3), "GBP").replace(QChar(0xA5), "JPY").replace('$', "USD").replace(QChar(0x20AC), "EUR").toLatin1();
    const char *ctext = ba.data();

    CALCULATOR->terminateThreads();
    EvaluationOptions eo;

    eo.auto_post_conversion = POST_CONVERSION_BEST;
    eo.keep_zero_units = false;

    eo.parse_options.angle_unit = ANGLE_UNIT_RADIANS;
    eo.structuring = STRUCTURING_SIMPLIFY;

    MathStructure result = CALCULATOR->calculate(ctext, eo);

    PrintOptions po;
    po.number_fraction_format = FRACTION_DECIMAL;
    po.indicate_infinite_series = false;
    po.use_all_prefixes = false;
    po.use_denominator_prefix = true;
    po.negative_exponents = false;
    po.lower_case_e = true;

    result.format(po);

    m_lastResult = result.print(po).c_str();

    return m_lastResult;
}

#include "moc_qalculate_engine.cpp"
