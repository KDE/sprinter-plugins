/***************************************************************************
 *   Copyright 2014 by Emmanuel Pescosta <emmanuelpescosta099@gmail.com>   *
 *   Copyright (C) 2008 by Dario Freddi <drf@kdemod.ath.cx>                *
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

#ifndef POWERDEVILRUNNER_H
#define POWERDEVILRUNNER_H

#include <Sprinter/Runner>

class PowerDevilRunner : public Sprinter::Runner
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.sprinter.powerdevil" FILE "powerdevil.json")
    Q_INTERFACES(Sprinter::Runner)

    enum PowerDevilWord {
        NoWord = -1,
        SuspendWord,
        SleepWord,
        HibernateWord,
        ToDiskWord,
        ToRamWord,
        ScreenBrightnessWord,
        DimScreenWord
    };

    enum PowerDevilAction {
        ChangeBrightnessAction,
        DimTotalAction,
        DimHalfAction,
        DimNotAction,
        SuspendAction,
        HibernateAction
    };

public:
    PowerDevilRunner(QObject *parent = nullptr);

    void match(Sprinter::MatchData &matchData) Q_DECL_OVERRIDE;
    bool exec(const Sprinter::QueryMatch &match) Q_DECL_OVERRIDE;

private:
    void addMatch(PowerDevilAction action, Sprinter::MatchData &matchData);
    void addBrightnessMatch(int brightness, Sprinter::MatchData &matchData);

private:
    QMap<PowerDevilAction, QIcon> m_actionIcons;
    QMap<PowerDevilWord, QString> m_words;
};

#endif // POWERDEVILRUNNER_H
