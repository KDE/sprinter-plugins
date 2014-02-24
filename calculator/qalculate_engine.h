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

#ifndef QALCULATEENGINE_H
#define QALCULATEENGINE_H

#include <QAtomicInt>
#include <QDateTime>
#include <QObject>
#include <QMutex>
#include <QNetworkAccessManager>

class QalculateEngine : public QObject
{
	Q_OBJECT
public:
	QalculateEngine(QObject* parent = 0);
	~QalculateEngine();

	QString lastResult() const { return m_lastResult; }

public slots:
	QString evaluate(const QString& expression);
	void updateExchangeRates();

protected slots:
    void exchangeRatesFetched();

signals:
	void resultReady(const QString&);
	void formattedResultReady(const QString&);

private:
	QString m_lastResult;
    QDateTime m_exchangeRatesUpdated;
    QTime m_lastAttempt;
    QNetworkAccessManager *m_network;
    QNetworkReply *m_networkReply;
    QMutex m_mutex;
	static QAtomicInt s_counter;
};

#endif // QALCULATEENGINE_H
