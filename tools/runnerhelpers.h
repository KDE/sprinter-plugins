/***************************************************************************
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

#ifndef BLOCKINGKRUN_H
#define BLOCKINGKRUN_H

#include <QEventLoop>
#include <QCoreApplication>

#ifdef HAVE_KRUN
#include <KRun>
#endif

namespace RunnerHelpers
{

#ifdef HAVE_KRUN
bool blockingKRun(const QString &command) {
    bool success = false;
    QEventLoop loop;
    KRun *krun = new KRun(command, nullptr);
    QObject::connect(krun, &KRun::finished,
                     [&]() { success = !krun->hasError(); loop.exit(); });
    krun->moveToThread(QCoreApplication::instance()->thread());
    loop.exec();
    return success;
}

bool blockingKRun(const QUrl &url) {
    bool success = false;
    QEventLoop loop;
    KRun *krun = new KRun(url, nullptr);
    QObject::connect(krun, &KRun::finished,
                     [&]() { success = !krun->hasError(); loop.exit(); });
    krun->moveToThread(QCoreApplication::instance()->thread());
    loop.exec();
    return success;
}
#endif

}

#endif