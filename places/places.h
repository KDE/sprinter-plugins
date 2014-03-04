/*
 *   Copyright 2014 Emmanuel Pescosta <emmanuelpescosta099@gmail.com>
 *   Copyright 2008 David Edmundson <kde@davidedmundson.co.uk>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#ifndef PLACESRUNNER_H
#define PLACESRUNNER_H

#include <Sprinter/Runner>

#include <KIOFileWidgets/KFilePlacesModel>

#include <QMutex>
#include <QWaitCondition>

class PlacesSessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    PlacesSessionData(Sprinter::Runner *runner);
    bool startExec(const Sprinter::QueryMatch &match);

private Q_SLOTS:
    void startQuery(const QString &query, const Sprinter::QueryContext &context);
    void requestSetup(const QString &deviceUdi);

private:
    void run(const QUrl &url);

    KFilePlacesModel m_places;
    QMutex m_mutex;
    QWaitCondition m_runWait;
    bool m_successfulRun;
};

class PlacesRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.places" FILE "places.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    PlacesRunner(QObject *parent = nullptr);

    Sprinter::RunnerSessionData *createSessionData();

    void match(Sprinter::MatchData &matchData) Q_DECL_OVERRIDE;
    bool exec(const Sprinter::QueryMatch &match) Q_DECL_OVERRIDE;

Q_SIGNALS:
    void startQuery(const QString &query, const Sprinter::QueryContext &context);

private:
    const QString m_placesWord;
};

#endif // PLACESRUNNER_H
