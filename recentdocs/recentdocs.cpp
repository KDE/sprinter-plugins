/*
 * Copyright (C) 2014 Aaron Seigo <aseigo@kde.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "recentdocs.h"

#include <QCoreApplication>
#include <QDebug>
#include <QEventLoop>
#include <QIcon>

#include <KConfig>
#include <KDesktopFile>
#include <KIOCore/KRecentDocument>
#include <KIOWidgets/KRun>
#include <KCoreAddons/KDirWatch>

RecentDocsSessionData::RecentDocsSessionData(Sprinter::Runner *runner)
    : Sprinter::RunnerSessionData(runner)
{
    updateRecentDocsList();

    KDirWatch *watch = new KDirWatch(this);
    watch->addDir(KRecentDocument::recentDocumentDirectory(),
                  KDirWatch::WatchFiles);
    connect(watch, &KDirWatch::created,
            this, &RecentDocsSessionData::updateRecentDocsList);
    connect(watch, &KDirWatch::deleted,
            this, &RecentDocsSessionData::updateRecentDocsList);
    connect(watch, &KDirWatch::dirty,
            this, &RecentDocsSessionData::updateRecentDocsList);
}

void RecentDocsSessionData::updateRecentDocsList()
{
    recentDocs.clear();
    QSet<QString> seen;
    RecentDoc doc;

    foreach (const QString desktopFile, KRecentDocument::recentDocuments()) {
        KDesktopFile file(desktopFile);
        doc.name = file.readName();
        if (seen.contains(doc.name)) {
            continue;
        }

        seen.insert(doc.name);
        doc.icon = file.readIcon();
        //TODO: if local, check to see if the file exists on disk
        doc.url = file.readUrl();
        recentDocs << doc;
    }
}

RecentDocsRunner::RecentDocsRunner(QObject *parent)
    : Sprinter::Runner(parent)
{
    setMatchTypesGenerated(QVector<Sprinter::QuerySession::MatchType>()
                                << Sprinter::QuerySession::FileType);
    setSourcesUsed(QVector<Sprinter::QuerySession::MatchSource>()
                        << Sprinter::QuerySession::FromFilesystem);
    setGeneratesDefaultMatches(true);
}

Sprinter::RunnerSessionData *RecentDocsRunner::createSessionData()
{
    return new RecentDocsSessionData(this);
}

void RecentDocsRunner::match(Sprinter::MatchData &matchData)
{
    RecentDocsSessionData *sessionData = qobject_cast<RecentDocsSessionData *>(matchData.sessionData());
    if (!sessionData || sessionData->recentDocs.isEmpty()) {
        return;
    }


    uint skipCount = 0;
    Sprinter::QueryContext context = matchData.queryContext();
    const bool listAll = context.isDefaultMatchesRequest();
    const QString term = context.query();
    for (int i = 0; i < sessionData->recentDocs.count(); ++i) {
        if (listAll ||
            sessionData->recentDocs[i].name.contains(term, Qt::CaseInsensitive)) {
            if (skipCount < matchData.sessionData()->resultsOffset()) {
                ++skipCount;
                continue;
            }

            if (matchData.matchCount() >= matchData.sessionData()->resultsPageSize()) {
                matchData.sessionData()->setCanFetchMoreMatches(true, context);
                return;
            }

            Sprinter::QueryMatch match;
            match.setTitle(sessionData->recentDocs[i].name);
            match.setText(tr("Recent Document"));
            match.setType(Sprinter::QuerySession::FileType);
            match.setSource(Sprinter::QuerySession::FromFilesystem);

            if (sessionData->recentDocs[i].name.compare(term, Qt::CaseInsensitive) == 0) {
                match.setPrecision(Sprinter::QuerySession::ExactMatch);
            } else {
                match.setPrecision(Sprinter::QuerySession::CloseMatch);
            }

            match.setImage(QIcon::fromTheme(sessionData->recentDocs[i].icon).pixmap(context.imageSize()).toImage());
            match.setUserData(sessionData->recentDocs[i].url);
            match.setData(sessionData->recentDocs[i].url);
            matchData << match;
        }
    }
}

bool RecentDocsRunner::exec(const Sprinter::QueryMatch &match)
{
    bool success = false;
    QEventLoop loop;
    KRun *krun = new KRun(match.data().toString(), 0, false);
    connect(krun, &KRun::finished,
            [&]() { success = !krun->hasError(); loop.exit(); });
    krun->moveToThread(QCoreApplication::instance()->thread());
    loop.exec();
    return success;
}

#include "moc_recentdocs.cpp"
