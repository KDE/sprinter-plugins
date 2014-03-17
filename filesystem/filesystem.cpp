/*
 * Copyright 2014  Aaron Seigo <aseigo@kde.org>
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


#include "filesystem.h"

#include <QDebug>
#include <QDesktopServices>
#include <QDir>
#include <QUrl>

// #include <QCoreApplication>

#include <KI18n/KLocalizedString>

#include "tools/runnerhelpers.h"

/*TODO:
paging
cache non-existent paths and skip checking them again and again
*/

FilesystemRunner::FilesystemRunner(QObject *parent)
        : Sprinter::Runner(parent),
          m_directoryIcon(QIcon::fromTheme("inode-directory"))
{
    setMinQueryLength(1);
}

FilesystemRunner::~FilesystemRunner()
{
}

void FilesystemRunner::match(Sprinter::MatchData &matchData)
{
    QString term = matchData.queryContext().query();
    QFileInfo info(term);
    if (info.exists()) {
        if (info.isDir()) {
            createDirectoryMatch(info, matchData, false);
        } else {
            createFileMatch(info, matchData, false);
        }

        return;
    }

    const int lastSlash = term.lastIndexOf('/');
    if (lastSlash == -1) {
        return;
    }

    QString fragment = term.right(term.length() - lastSlash - 1) + '*';
    term = term.left(lastSlash + 1);

    QDir dir(term);
    for (auto entry: dir.entryInfoList(QStringList() << fragment)) {
        if (entry.isDir()) {
            createDirectoryMatch(entry, matchData, true);
        } else {
            createFileMatch(entry, matchData, true);
        }
    }
}

bool FilesystemRunner::exec(const Sprinter::QueryMatch &match)
{
    return RunnerHelpers::blockingKRun(match.data().toString());
}


void FilesystemRunner::createDirectoryMatch(const QFileInfo &info, Sprinter::MatchData &matchData, bool completion)
{
    Sprinter::QueryMatch match;
    match.setTitle(i18n("Open %1", info.fileName()));
    match.setImage(generateImage(m_directoryIcon, matchData.queryContext()));
    const QString fullPath = info.canonicalFilePath();
    match.setUserData(fullPath);
    match.setData(fullPath);
    match.setText(fullPath);
    match.setType(Sprinter::QuerySession::FileType);
    match.setSource(Sprinter::QuerySession::FromFilesystem);
    match.setPrecision(completion ? Sprinter::QuerySession::CloseMatch : Sprinter::QuerySession::ExactMatch);
    matchData << match;
}

void FilesystemRunner::createFileMatch(const QFileInfo &info, Sprinter::MatchData &matchData, bool completion)
{
    Sprinter::QueryMatch match;
    match.setTitle(i18n("Open %1", info.fileName()));

    QMimeType mimetype = m_mimetypes.mimeTypeForFile(info);
    match.setImage(generateImage(QIcon::fromTheme(mimetype.iconName()), matchData.queryContext()));
    const QString fullPath = info.canonicalFilePath();
    match.setUserData(fullPath);
    match.setData(fullPath);
    match.setText(fullPath);
    match.setType(Sprinter::QuerySession::FileType);
    match.setSource(Sprinter::QuerySession::FromFilesystem);
    match.setPrecision(completion ? Sprinter::QuerySession::CloseMatch : Sprinter::QuerySession::ExactMatch);
    matchData << match;
}

#include "moc_filesystem.cpp"
