/***************************************************************************
 *   Copyright 2014 by Emmanuel Pescosta <emmanuelpescosta099@gmail.com>   *
 *   Copyright 2008 by Dario Freddi <drf@kdemod.ath.cx>                    *
 *   Copyright 2008 by Sebastian Kügler <sebas@kde.org>                    *
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

#include "powerdevil.h"

#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusInterface>

#include <Solid/PowerManagement>
#include <KI18n/KLocalizedString>

static const QChar ParameterSeperator('_');

PowerDevilRunner::PowerDevilRunner(QObject *parent)
    : Runner(parent)
{
    m_actionIcons.insert(ChangeBrightnessAction, QIcon::fromTheme("preferences-system-power-management"));
    m_actionIcons.insert(DimTotalAction,         QIcon::fromTheme("preferences-system-power-management"));
    m_actionIcons.insert(DimHalfAction,          QIcon::fromTheme("preferences-system-power-management"));
    m_actionIcons.insert(DimNotAction ,          QIcon::fromTheme("preferences-system-power-management"));
    m_actionIcons.insert(SuspendAction,          QIcon::fromTheme("system-suspend"));
    m_actionIcons.insert(HibernateAction,        QIcon::fromTheme("system-suspend-hibernate"));

    m_words.insert(SuspendWord,          i18n("suspend"));
    m_words.insert(SleepWord,            i18n("sleep"));
    m_words.insert(HibernateWord,        i18n("hibernate"));
    m_words.insert(ToDiskWord,           i18n("to disk"));
    m_words.insert(ToRamWord,            i18n("to ram"));
    m_words.insert(ScreenBrightnessWord, i18n("screen brightness"));
    m_words.insert(DimScreenWord,        i18n("dim screen"));

    // find the shortest word length
    int minQueryLength = 100;
    for (auto it = m_words.cbegin(); it != m_words.cend(); ++it) {
        if (it.value().length() < minQueryLength) {
            minQueryLength = it.value().length();
        }
    }

    setMinQueryLength(minQueryLength);
}

void PowerDevilRunner::match(Sprinter::MatchData &matchData)
{
    const QString term = matchData.queryContext().query();

    PowerDevilWord word = NoWord;
    for (auto it = m_words.cbegin(); it != m_words.cend(); ++it) {
        if (term.startsWith(it.value(), Qt::CaseInsensitive)) {
            word = it.key();
            break;
        }
    }

    // extracts the parameters from the given term
    auto getParameters = [&term](const QString &word) -> QStringList {
        QString adjustedTerm = term;
        adjustedTerm.remove(0, word.length());
        return adjustedTerm.trimmed().split(' ', QString::SkipEmptyParts);
    };

    switch (word) {
    case SuspendWord: {
        const QSet<Solid::PowerManagement::SleepState> states = Solid::PowerManagement::supportedSleepStates();
        if (states.contains(Solid::PowerManagement::SuspendState)) {
            addMatch(SuspendAction, matchData);
        }

        if (states.contains(Solid::PowerManagement::HibernateState)) {
            addMatch(HibernateAction, matchData);
        }
    } break;

    case SleepWord:
    case ToRamWord:
        addMatch(SuspendAction, matchData);
        break;

    case HibernateWord:
    case ToDiskWord:
        addMatch(HibernateAction, matchData);
        break;

    case ScreenBrightnessWord:
    case DimScreenWord: {
        const QStringList parameters = getParameters(m_words.value(word));

        int brightness;
        bool brightnessValid = false;
        if (!parameters.isEmpty()) {
            brightness = parameters.first().toInt(&brightnessValid);
        }

        if (brightnessValid) {
            addBrightnessMatch(brightness, matchData);
        } else {
            addMatch(DimTotalAction, matchData);
            addMatch(DimHalfAction, matchData);
            addMatch(DimNotAction, matchData);
        }
    } break;

    default:
        // No match found
        break;
    }
}

void PowerDevilRunner::addMatch(PowerDevilRunner::PowerDevilAction action, Sprinter::MatchData &matchData)
{
    Sprinter::QueryMatch match;

    switch (action) {
    case DimTotalAction:
        match.setText(i18n("Dim screen completely"));
        break;

    case DimHalfAction:
        match.setText(i18n("Dim screen halfway"));
        break;

    case DimNotAction:
        match.setText(i18n("Make screen bright"));
        break;

    case SuspendAction:
        match.setText(i18n("Suspend to RAM"));
        break;

    case HibernateAction:
        match.setText(i18n("Suspend to Disk"));
        break;

    case ChangeBrightnessAction:
        Q_ASSERT_X(false, "PowerDevilRunner::addMatch", "Please use PowerDevilRunner::addBrightnessMatch");
        return;

    default:
        Q_ASSERT_X(false, "PowerDevilRunner::addMatch", "PowerDevilAction type does not exist");
        return;
    }

    match.setType(Sprinter::QuerySession::HardwareType);
    match.setSource(Sprinter::QuerySession::FromLocalService);
    match.setPrecision(Sprinter::QuerySession::ExactMatch);
    match.setImage(generateImage(m_actionIcons.value(action), matchData.queryContext()));

    QStringList data;
    data.append(QLatin1String("PowerDevilAction"));
    data.append(QString::number((int)action));
    match.setData(data.join(ParameterSeperator));

    matchData << match;
}

void PowerDevilRunner::addBrightnessMatch(int brightness, Sprinter::MatchData &matchData)
{
    brightness = qBound(0, brightness, 100);

    Sprinter::QueryMatch match;
    match.setType(Sprinter::QuerySession::HardwareType);
    match.setSource(Sprinter::QuerySession::FromLocalService);
    match.setImage(generateImage(m_actionIcons.value(ChangeBrightnessAction), matchData.queryContext()));
    match.setText(i18n("Set Brightness to %1", brightness));
    match.setPrecision(Sprinter::QuerySession::ExactMatch);

    QStringList data;
    data.append(QLatin1String("PowerDevilAction"));
    data.append(QString::number((int)ChangeBrightnessAction));
    data.append(QLatin1String("Brightness"));
    data.append(QString::number(brightness));
    match.setData(data.join(ParameterSeperator));

    matchData << match;
}

bool PowerDevilRunner::exec(const Sprinter::QueryMatch &match)
{
    const QStringList parts = match.data().toString().split(ParameterSeperator, QString::SkipEmptyParts);
    if (parts.size() < 2) {
        return false;
    }
    Q_ASSERT(parts.first() == QLatin1String("PowerDevilAction"));
    const PowerDevilAction action = PowerDevilAction(parts.at(1).toInt());

    auto setBrightness = [](int brightness) {
        QDBusInterface iface("org.kde.Solid.PowerManagement",
                             "/org/kde/Solid/PowerManagement/Actions/BrightnessControl",
                             "org.kde.Solid.PowerManagement.Actions.BrightnessControl");
        iface.asyncCall("setBrightness", brightness);
    };

    switch (action) {
    case ChangeBrightnessAction:
        if (parts.size() < 4) {
            return false;
        }
        Q_ASSERT(parts.at(2) == QLatin1String("Brightness"));
        setBrightness(parts.at(3).toInt());
        break;

    case DimTotalAction:
        setBrightness(0);
        break;

    case DimHalfAction:
        setBrightness(50);
        break;

    case DimNotAction:
        setBrightness(100);
        break;

    case PowerDevilRunner::SuspendAction:
        Solid::PowerManagement::requestSleep(Solid::PowerManagement::SuspendState, 0, 0);
        break;

    case PowerDevilRunner::HibernateAction:
        Solid::PowerManagement::requestSleep(Solid::PowerManagement::HibernateState, 0, 0);
        break;

    default:
        Q_ASSERT_X(false, "PowerDevilRunner::exec", "PowerDevilAction type does not exist");
        return false;
    }

    return true;
}

#include "powerdevil.moc"
