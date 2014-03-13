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

#include "windows.h"

#include <QDebug>
#include <QX11Info>

#include <KI18n/KLocalizedString>
#include <KWindowSystem>
#include <NETWM>

WindowsRunner::WindowsRunner(QObject* parent)
    : Sprinter::Runner(parent),
      m_desktopIcon(QIcon::fromTheme("user-desktop"))
{
}

WindowsRunner::~WindowsRunner()
{
}

template<typename Func>
void WindowsRunner::forEachWindow(Func algorithm) const
{
    for (auto w: KWindowSystem::windows()) {
        KWindowInfo info = KWindowInfo(w, NET::WMWindowType | NET::WMDesktop |
                                       NET::WMState | NET::XAWMState |
                                       NET::WMName,
                                       NET::WM2WindowClass | NET::WM2WindowRole |
                                       NET::WM2AllowedActions);
        if (info.valid()) {
            // ignore NET::Tool and other special window types
            NET::WindowType wType = info.windowType(NET::NormalMask | NET::DesktopMask |
                                                    NET::DockMask |
                                                    NET::ToolbarMask | NET::MenuMask |
                                                    NET::DialogMask |
                                                    NET::OverrideMask | NET::TopMenuMask |
                                                    NET::UtilityMask | NET::SplashMask);

            if (wType != NET::Normal && wType != NET::Override && wType != NET::Unknown &&
                wType != NET::Dialog && wType != NET::Utility) {
                continue;
            }

            algorithm(info);
        }
    }
}

void WindowsRunner::match(Sprinter::MatchData &matchData)
{
    QString term = matchData.queryContext().query();

    // check if the search term ends with an action keyword
    WindowAction action = ActivateAction;
    if (term.endsWith(i18n("activate") , Qt::CaseInsensitive)) {
        action = ActivateAction;
        term = term.left(term.lastIndexOf(i18n("activate")) - 1);
    } else if (term.endsWith(i18n("close") , Qt::CaseInsensitive)) {
        action = CloseAction;
        term = term.left(term.lastIndexOf(i18n("close")) - 1);
    } else if (term.endsWith(i18n("min") , Qt::CaseInsensitive)) {
        action = MinimizeAction;
        term = term.left(term.lastIndexOf(i18n("min")) - 1);
    } else if (term.endsWith(i18n("minimize") , Qt::CaseInsensitive)) {
        action = MinimizeAction;
        term = term.left(term.lastIndexOf(i18n("minimize")) - 1);
    } else if (term.endsWith(i18n("max") , Qt::CaseInsensitive)) {
        action = MaximizeAction;
        term = term.left(term.lastIndexOf(i18n("max")) - 1);
    } else if (term.endsWith(i18n("maximize") , Qt::CaseInsensitive)) {
        action = MaximizeAction;
        term = term.left(term.lastIndexOf(i18n("maximize")) - 1);
    } else if (term.endsWith(i18n("fullscreen") , Qt::CaseInsensitive)) {
        action = FullscreenAction;
        term = term.left(term.lastIndexOf(i18n("fullscreen")) - 1);
    } else if (term.endsWith(i18n("shade") , Qt::CaseInsensitive)) {
        action = ShadeAction;
        term = term.left(term.lastIndexOf(i18n("shade")) - 1);
    } else if (term.endsWith(i18n("keep above") , Qt::CaseInsensitive)) {
        action = KeepAboveAction;
        term = term.left(term.lastIndexOf(i18n("keep above")) - 1);
    } else if (term.endsWith(i18n("keep below") , Qt::CaseInsensitive)) {
        action = KeepBelowAction;
        term = term.left(term.lastIndexOf(i18n("keep below")) - 1);
    }

    // keyword match: when term starts with "window" we list all windows
    // the list can be restricted to windows matching a given name, class, role or desktop
    if (term.startsWith(i18n("window") , Qt::CaseInsensitive)) {
        const QStringList keywords = term.split(" ");
        QString windowName;
        QString windowClass;
        QString windowRole;
        int desktop = -1;
        bool found = false;

        foreach (const QString& keyword, keywords) {
            if (keyword.endsWith('=')) {
                continue;
            }
            if (keyword.startsWith(i18n("name") + "=" , Qt::CaseInsensitive)) {
                windowName = keyword.split("=")[1];
            } else if (keyword.startsWith(i18n("class") + "=" , Qt::CaseInsensitive)) {
                windowClass = keyword.split("=")[1];
            } else if (keyword.startsWith(i18n("role") + "=" , Qt::CaseInsensitive)) {
                windowRole = keyword.split("=")[1];
            } else if (keyword.startsWith(i18n("desktop") + "=" , Qt::CaseInsensitive)) {
                bool ok;
                desktop = keyword.split("=")[1].toInt(&ok);
                if (!ok || desktop > KWindowSystem::numberOfDesktops()) {
                    desktop = -1; // sanity check
                }
            } else {
                // not a keyword - use as name if name is unused, but another option is set
                if (windowName.isEmpty() && !keyword.contains('=') &&
                    (!windowRole.isEmpty() || !windowClass.isEmpty() || desktop != -1)) {
                    windowName = keyword;
                }
            }
        }


        auto matchWindow = [&](const KWindowInfo &info) {
            QString windowClassCompare = QString::fromUtf8(info.windowClassName()) + " " +
                                         QString::fromUtf8(info.windowClassClass());
            // exclude not matching windows
            if (!windowName.isEmpty() && !info.name().contains(windowName, Qt::CaseInsensitive)) {
                return;
            }

            if (!windowClass.isEmpty() && !windowClassCompare.contains(windowClass, Qt::CaseInsensitive)) {
                return;
            }

            if (!windowRole.isEmpty() && !QString::fromUtf8(info.windowRole()).contains(windowRole, Qt::CaseInsensitive)) {
                return;
            }

            if (desktop != -1 && !info.isOnDesktop(desktop)) {
                return;
            }

            // check for windows when no keywords were used
            // check the name, class and role for containing the query without the keyword
            if (windowName.isEmpty() && windowClass.isEmpty() && windowRole.isEmpty() && desktop == -1) {
                const QString& test = term.mid(keywords[0].length() + 1);
                if (!info.name().contains(test, Qt::CaseInsensitive) &&
                    !windowClassCompare.contains(test, Qt::CaseInsensitive) &&
                    !QString::fromUtf8(info.windowRole()).contains(test, Qt::CaseInsensitive)) {
                    return;
                }
            }

            // blacklisted everything else: we have a match
            if (actionSupported(info, action)){
                addWindowMatch(info, action,
                               Sprinter::QuerySession::ExactMatch,
                               matchData);
                found = true;
            }
        };

        forEachWindow(matchWindow);

        if (found) {
            // the window keyword found matches - do not process other syntax possibilities
            return;
        }
    }
    bool desktopAdded = false;
    // check for desktop keyword
    if (term.startsWith(i18n("desktop") , Qt::CaseInsensitive)) {
        const QStringList parts = term.split(" ");
        if (parts.size() == 1) {
            // only keyword - list all desktops
            for (int i=1; i<=KWindowSystem::numberOfDesktops(); i++) {
                if (i == KWindowSystem::currentDesktop()) {
                    continue;
                }
                addDesktopMatch(i, Sprinter::QuerySession::ExactMatch, matchData);
                desktopAdded = true;
            }
        } else {
            // keyword + desktop - restrict matches
            bool isInt;
            int desktop = term.mid(parts[0].length() + 1).toInt(&isInt);
            if (isInt && desktop != KWindowSystem::currentDesktop()) {
                addDesktopMatch(desktop, Sprinter::QuerySession::ExactMatch, matchData);
                desktopAdded = true;
            }
        }
    }

    // check for matches without keywords
    auto matchWindow = [&](const KWindowInfo &info) {
        // check if window name, class or role contains the query
        QString className = QString::fromUtf8(info.windowClassName());
        if (info.name().startsWith(term, Qt::CaseInsensitive) ||
            className.startsWith(term, Qt::CaseInsensitive)) {
            addWindowMatch(info, action,
                           Sprinter::QuerySession::CloseMatch,
                           matchData);
        } else if ((info.name().contains(term, Qt::CaseInsensitive) ||
                   className.contains(term, Qt::CaseInsensitive)) &&
            actionSupported(info, action)) {
            addWindowMatch(info, action,
                           Sprinter::QuerySession::FuzzyMatch,
                           matchData);
        }
    };

    forEachWindow(matchWindow);

    // check for matching desktops by name
    for (int i = 1; i <= KWindowSystem::numberOfDesktops(); ++i) {
        const QString desktopName = KWindowSystem::desktopName(i);
        if (desktopName.contains(term, Qt::CaseInsensitive)) {
            // desktop name matches - offer switch to
            // only add desktops if it hasn't been added by the keyword which is quite likely
            if (!desktopAdded && i != KWindowSystem::currentDesktop()) {
                addDesktopMatch(i, Sprinter::QuerySession::CloseMatch, matchData);
            }

            // search for windows on desktop and list them with less relevance
            auto matchWindowsOnDesktop = [&](const KWindowInfo &info) {
                if (info.isOnDesktop(i) && actionSupported(info, action)) {
                    addWindowMatch(info, action,
                                   Sprinter::QuerySession::FuzzyMatch,
                                   matchData);
                }
            };

            forEachWindow(matchWindowsOnDesktop);
        }
    }
}

bool WindowsRunner::exec(const Sprinter::QueryMatch& match)
{
    // check if it's a desktop
    if (match.type() == Sprinter::QuerySession::DesktopType) {
        KWindowSystem::setCurrentDesktop(match.data().toInt());
        return true;
    }

    const QStringList parts = match.data().toString().split("_");
    WindowAction action = WindowAction(parts[0].toInt());
    WId w = WId(parts[1].toULong());
    KWindowInfo info = KWindowInfo(w, NET::WMState);
    switch (action) {
    case ActivateAction:
        KWindowSystem::forceActiveWindow(w);
        break;
    case CloseAction:
        {
        NETRootInfo ri(QX11Info::connection(), NET::CloseWindow);
        ri.closeWindowRequest(w);
        break;
        }
    case MinimizeAction:
        if (info.isMinimized()) {
            KWindowSystem::unminimizeWindow(w);
        } else {
            KWindowSystem::minimizeWindow(w);
        }
        break;
    case MaximizeAction:
        if (info.hasState(NET::Max)) {
            KWindowSystem::clearState(w, NET::Max);
        } else {
            KWindowSystem::setState(w, NET::Max);
        }
        break;
    case FullscreenAction:
        if (info.hasState(NET::FullScreen)) {
            KWindowSystem::clearState(w, NET::FullScreen);
        } else {
            KWindowSystem::setState(w, NET::FullScreen);
        }
        break;
    case ShadeAction:
        if (info.hasState(NET::Shaded)) {
            KWindowSystem::clearState(w, NET::Shaded);
        } else {
            KWindowSystem::setState(w, NET::Shaded);
        }
        break;
    case KeepAboveAction:
        if (info.hasState(NET::KeepAbove)) {
            KWindowSystem::clearState(w, NET::KeepAbove);
        } else {
            KWindowSystem::setState(w, NET::KeepAbove);
        }
        break;
    case KeepBelowAction:
        if (info.hasState(NET::KeepBelow)) {
            KWindowSystem::clearState(w, NET::KeepBelow);
        } else {
            KWindowSystem::setState(w, NET::KeepBelow);
        }
        break;
    }

    return true;
}

void WindowsRunner::addDesktopMatch(int desktop,
                                    Sprinter::QuerySession::MatchPrecision precision,
                                    Sprinter::MatchData &matchData)
{
    Sprinter::QueryMatch match;
    match.setType(Sprinter::QuerySession::DesktopType);
    match.setSource(Sprinter::QuerySession::FromDesktopShell);
    match.setData(desktop);
    match.setImage(generateImage(m_desktopIcon, matchData.queryContext()));
    QString desktopName = KWindowSystem::desktopName(desktop);
    match.setTitle(desktopName);
    match.setText(i18n("Switch to desktop ").arg(desktop));
    match.setPrecision(precision);
    matchData << match;
}

void WindowsRunner::addWindowMatch(const KWindowInfo& info,
                                   WindowAction action,
                                   Sprinter::QuerySession::MatchPrecision precision,
                                   Sprinter::MatchData &matchData)
{
    Sprinter::QueryMatch match;
    match.setType(Sprinter::QuerySession::WindowType);
    match.setSource(Sprinter::QuerySession::FromDesktopShell);
    match.setData(QString(QString::number((int)action) + "_" + QString::number(info.win())));
    const QSize imageSize = matchData.queryContext().imageSize();
    //TODO: cache the images?
    QImage icon = KWindowSystem::icon(info.win(), imageSize.width(), imageSize.height()).scaled(imageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation).toImage();
    match.setImage(icon);
    match.setTitle(info.name());

    int desktop = info.desktop();
    if (desktop == NET::OnAllDesktops) {
        desktop = KWindowSystem::currentDesktop();
    }

    QString desktopName = KWindowSystem::desktopName(desktop);

    switch (action) {
    case CloseAction:
        match.setText(i18n("Close running window on ").arg(desktopName));
        break;
    case MinimizeAction:
        match.setText(i18n("(Un)minimize running window on ").arg(desktopName));
        break;
    case MaximizeAction:
        match.setText(i18n("Maximize/restore running window on ").arg(desktopName));
        break;
    case FullscreenAction:
        match.setText(i18n("Toggle fullscreen for running window on ").arg(desktopName));
        break;
    case ShadeAction:
        match.setText(i18n("(Un)shade running window on ").arg(desktopName));
        break;
    case KeepAboveAction:
        match.setText(i18n("Toggle keep above for running window on ").arg(desktopName));
        break;
    case KeepBelowAction:
        match.setText(i18n("Toggle keep below running window on ").arg(desktopName));
        break;
    case ActivateAction:
    default:
        match.setText(i18n("Activate running window on ").arg(desktopName));
        break;
    }
    match.setPrecision(precision);
    matchData << match;
}

bool WindowsRunner::actionSupported(const KWindowInfo& info, WindowAction action)
{
    switch (action) {
    case CloseAction:
        return info.actionSupported(NET::ActionClose);
    case MinimizeAction:
        return info.actionSupported(NET::ActionMinimize);
    case MaximizeAction:
        return info.actionSupported(NET::ActionMax);
    case ShadeAction:
        return info.actionSupported(NET::ActionShade);
    case FullscreenAction:
        return info.actionSupported(NET::ActionFullScreen);
    case KeepAboveAction:
    case KeepBelowAction:
    case ActivateAction:
    default:
        return true;
    }
}

#include "moc_windows.cpp"
