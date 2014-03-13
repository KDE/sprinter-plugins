/* Copyright 2009  Jan Gerrit Marker <jangerrit@weiler-marker.com>
 * Copyright 2014  Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) version 3, or any
 * later version accepted by the membership of KDE e.V. (or its
 * successor approved by the membership of KDE e.V.), which shall
 * act as a proxy defined in Section 6 of version 3 of the license.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kill.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QProcess>

#include <KUser>
#include <KAuth>

#include "ksysguard/processes.h"
#include "ksysguard/process.h"

KillSessionData::KillSessionData(Sprinter::Runner *runner)
    : Sprinter::RunnerSessionData(runner),
      m_processes(0)
{
}

KillSessionData::~KillSessionData()
{
    delete m_processes;
}

void KillSessionData::updateProcessTable()
{
    const bool update = !m_processes || m_lastUpdate.elapsed() > 5000;
    if (!m_processes) {
        m_processes = new KSysGuard::Processes;
    }

    if (update) {
        m_processes->updateAllProcesses();
        m_lastUpdate.restart();
    }
}

KillRunner::KillRunner(QObject *parent)
        : Sprinter::Runner(parent),
          m_triggerWord(i18n("kill ")),
          m_icon(QIcon::fromTheme("application-exit"))
{
}

KillRunner::~KillRunner()
{
}

Sprinter::RunnerSessionData *KillRunner::createSessionData()
{
    return new KillSessionData(this);
}

void KillRunner::match(Sprinter::MatchData &matchData)
{
    KillSessionData *sessionData = qobject_cast<KillSessionData *>(matchData.sessionData());
    if (!sessionData) {
        return;
    }

    QString term = matchData.queryContext().query();
    if (!term.startsWith(m_triggerWord, Qt::CaseInsensitive)) {
        return;
    }

    QMetaObject::invokeMethod(matchData.sessionData(), "updateProcessTable",
                              Qt::BlockingQueuedConnection);
    term = term.right(term.length() - m_triggerWord.length());

    if (term.length() < 2)  {
        return;
    }
    const QList<KSysGuard::Process *> processlist = sessionData->m_processes->getAllProcesses();
    for (auto process: processlist) {
        if (!matchData.isValid()) {
            return;
        }

        const QString name = process->name;
        if (!name.contains(term, Qt::CaseInsensitive)) {
            //Process doesn't match the search term
            continue;
        }

        const quint64 pid = process->pid;
        const qlonglong uid = process->uid;
        const QString user = getUserName(uid);

        QVariantList data;
        data << pid << user;

        Sprinter::QueryMatch match;
        match.setTitle(i18n("Terminate %1", name));
        match.setText(i18n("Process ID: %1\nRunning as user: %2", QString::number(pid), user));
        match.setImage(generateImage(m_icon, matchData.queryContext()));
        match.setUserData(QStringLiteral("kill -9 ") + pid);
        match.setData(data);
        match.setType(Sprinter::QuerySession::AppActionType);
        match.setSource(Sprinter::QuerySession::FromLocalService);
        matchData << match;
    }
}

bool KillRunner::exec(const Sprinter::QueryMatch &match)
{
    QVariantList data = match.data().value<QVariantList>();
    quint64 pid = data[0].toUInt();
    QString user = data[1].toString();

    int signal = 9; //default: SIGKILL

    QStringList args;
    args << QString("-%1").arg(signal) << QString("%1").arg(pid);
    QProcess process;
    process.setProgram(QStringLiteral("kill"));
    process.setArguments(args);
    process.start();
    process.waitForFinished();
    if (process.exitCode() == 0) {
        return true;
    }

    KAuth::Action *killAction =
        new KAuth::Action("org.kde.ksysguard.processlisthelper.sendsignal");
    killAction->setHelperId("org.kde.ksysguard.processlisthelper");
    killAction->addArgument("pid0", pid);
    killAction->addArgument("pidcount", 1);
    killAction->addArgument("signal", signal);

    bool success = false;
    QEventLoop loop;
    KAuth::ExecuteJob *job = killAction->execute();
    job->moveToThread(QCoreApplication::instance()->thread());
    connect(job, &KJob::finished,
            [&]() { success = !job->error(); loop.exit(); });
    loop.exec();
    return success;
}

QString KillRunner::getUserName(qlonglong uid)
{
    KUser user(uid);
    return user.isValid() ? user.loginName() : "root";
}

#include "moc_kill.cpp"
