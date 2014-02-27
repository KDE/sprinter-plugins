/***************************************************************************
 *   Copyright 2009 by Martin Gräßlin <kde@martin-graesslin.com>           *
 *   Copyright 2014 by Aaron Seigo <aseigo@kde.org>                        *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/
#ifndef WINDOWSRUNNER_H
#define WINDOWSRUNNER_H

#include <Sprinter/Runner>

#include <QIcon>

class KWindowInfo;

class WindowsSessionData : public Sprinter::RunnerSessionData
{
    Q_OBJECT

public:
    WindowsSessionData(Sprinter::Runner *runner);

    QHash<WId, KWindowInfo> m_windows;
    QHash<WId, QIcon> m_icons;
    QStringList m_desktopNames;

private Q_SLOTS:
    void performUpdate();
};

class WindowsRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.windows" FILE "windows.json")
    Q_INTERFACES(Sprinter::Runner)

public:
    WindowsRunner(QObject *parent = 0);
    ~WindowsRunner();

    Sprinter::RunnerSessionData *createSessionData();
    virtual void match(Sprinter::MatchData &context);
    virtual bool exec(const Sprinter::QueryMatch &match);

private Q_SLOTS:
    void prepareForMatchSession();
    void matchSessionComplete();
    void gatherInfo();

private:
    enum WindowAction {
        ActivateAction,
        CloseAction,
        MinimizeAction,
        MaximizeAction,
        FullscreenAction,
        ShadeAction,
        KeepAboveAction,
        KeepBelowAction
    };
    void addDesktopMatch(int desktop,
                         Sprinter::QuerySession::MatchPrecision precision,
                         Sprinter::MatchData &context);
    void addWindowMatch(const KWindowInfo& info, WindowAction action,
                        Sprinter::QuerySession::MatchPrecision precision,
                        Sprinter::MatchData &context);
    bool actionSupported(const KWindowInfo& info, WindowAction action);

    QIcon m_desktopIcon;
};

#endif // WINDOWSRUNNER_H
