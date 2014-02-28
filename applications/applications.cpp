/*
 *   Copyright (C) 2006-2014 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "applications.h"

#include <QDebug>
#include <QIcon>

#include <KRun>
#include <KService>
#include <KServiceGroup>
#include <KServiceTypeTrader>

static const QString s_groupSearchKeyword("_groupRelPath");

ApplicationsRunner::ApplicationsRunner(QObject *parent)
    : Sprinter::Runner(parent)
{
    setMatchTypesGenerated(QVector<Sprinter::QuerySession::MatchType>()
                                << Sprinter::QuerySession::ExecutableType);
    setSourcesUsed(QVector<Sprinter::QuerySession::MatchSource>()
                        << Sprinter::QuerySession::FromFilesystem);
    setGeneratesDefaultMatches(true);
    setMinQueryLength(1);
}

ApplicationsRunner::~ApplicationsRunner()
{
}

void ApplicationsRunner::generateTopLevelGroups(Sprinter::MatchData &matchData,
                                                const Sprinter::QueryContext &context)
{
    KServiceGroup::Ptr rootGroup = KServiceGroup::root();
    uint skipCount = 0;

    foreach (KServiceGroup::Ptr group, rootGroup->groupEntries()) {
        if (skipCount < matchData.sessionData()->resultsOffset()) {
            ++skipCount;
            continue;
        }

        if (matchData.matchCount() >= matchData.sessionData()->resultsPageSize()) {
            return;
        }

        Sprinter::QueryMatch match;

        match.setTitle(group->caption());
        match.setText(group->comment());
        match.setData(s_groupSearchKeyword + group->relPath());
        match.setType(Sprinter::QuerySession::ExecutableType);
        match.setPrecision(Sprinter::QuerySession::ExactMatch);
        match.setSource(Sprinter::QuerySession::FromFilesystem);
        match.setIsSearchTerm(true);

        if (!group->icon().isEmpty()) {
            match.setImage(generateImage(QIcon::fromTheme(group->icon()), context));
        }

        matchData << match;
    }
}

void ApplicationsRunner::showEntriesInGroup(const QString &relPath,
                                            Sprinter::MatchData &matchData,
                                            const Sprinter::QueryContext &context)
{
    KServiceGroup::Ptr parentGroup = KServiceGroup::group(relPath);
    uint skipCount = 0;
    foreach (KServiceGroup::Ptr group, parentGroup->groupEntries()) {
        if (skipCount < matchData.sessionData()->resultsOffset()) {
            ++skipCount;
            continue;
        }

        if (matchData.matchCount() >= matchData.sessionData()->resultsPageSize()) {
            return;
        }

        Sprinter::QueryMatch match;

        match.setTitle(group->caption());
        match.setText(group->comment());
        match.setData(s_groupSearchKeyword + group->relPath());
        match.setType(Sprinter::QuerySession::ExecutableType);
        match.setPrecision(Sprinter::QuerySession::ExactMatch);
        match.setSource(Sprinter::QuerySession::FromFilesystem);
        match.setIsSearchTerm(true);

        if (!group->icon().isEmpty()) {
            match.setImage(generateImage(QIcon::fromTheme(group->icon()), context));
        }

        matchData << match;
    }

    KService::List services = parentGroup->serviceEntries();
    foreach (const KService::Ptr &service, services) {
        if (skipCount < matchData.sessionData()->resultsOffset()) {
            ++skipCount;
            continue;
        }

        if (matchData.matchCount() >= matchData.sessionData()->resultsPageSize()) {
            return;
        }

        Sprinter::QueryMatch match;
        match.setPrecision(Sprinter::QuerySession::ExactMatch);
        setupMatch(service, match, context);
        matchData << match;
    }
}

void ApplicationsRunner::match(Sprinter::MatchData &matchData)
{
    Sprinter::QueryContext context = matchData.queryContext();
    if (context.isDefaultMatchesRequest()) {
        generateTopLevelGroups(matchData, context);
        return;
    }

    QString term = context.query();

    if (term.startsWith(s_groupSearchKeyword)) {
        term = term.right(term.length() - s_groupSearchKeyword.length());
        showEntriesInGroup(term, matchData, context);
        return;
    }

    QSet<QString> seen;
    QString query;
    uint skipCount = 0;

    // Query syntax doc:
    // http://techbase.kde.org/Development/Tutorials/Services/Traders#The_KTrader_Query_Language
    if (term.length() > 1) {
        // Search for applications which are executable and case-insensitively match the search term

        query = QString("exist Exec and ('%1' =~ Name)").arg(term);
        KService::List services = KServiceTypeTrader::self()->query("Application", query);

        if (!services.isEmpty()) {
            //qDebug() << service->name() << "is an exact match!" << service->storageId() << service->exec();
            foreach (const KService::Ptr &service, services) {
                if (skipCount < matchData.sessionData()->resultsOffset()) {
                    ++skipCount;
                    continue;
                }

                if (matchData.matchCount() >= matchData.sessionData()->resultsPageSize()) {
                    return;
                }

                if (!service->noDisplay() && service->property("NotShowIn", QVariant::String) != "KDE") {
                    Sprinter::QueryMatch match;
                    setupMatch(service, match, context);
                    match.setPrecision(Sprinter::QuerySession::ExactMatch);
                    matchData << match;
                    seen.insert(service->storageId());
                    seen.insert(service->exec());
                }
            }
        }
    }

    if (!context.isValid()) {
        return;
    }

    // If the term length is < 3, no real point searching the Keywords and GenericName
    if (term.length() < 3) {
        query = QString("exist Exec and ( (exist Name and '%1' ~~ Name) or ('%1' ~~ Exec) )").arg(term);
    } else {
        // Search for applications which are executable and the term case-insensitive matches any of
        // * a substring of one of the keywords
        // * a substring of the GenericName field
        // * a substring of the Name field
        // Note that before asking for the content of e.g. Keywords and GenericName we need to ask if
        // they exist to prevent a tree evaluation error if they are not defined.
        query = QString("exist Exec and ( (exist Keywords and '%1' ~subin Keywords) or (exist GenericName and '%1' ~~ GenericName) or (exist Name and '%1' ~~ Name) or ('%1' ~~ Exec) )").arg(term);
    }

    KService::List services = KServiceTypeTrader::self()->query("Application", query);
    services += KServiceTypeTrader::self()->query("KCModule", query);

    //qDebug() << "got " << services.count() << " services from " << query;
    foreach (const KService::Ptr &service, services) {
        if (skipCount < matchData.sessionData()->resultsOffset()) {
            ++skipCount;
            continue;
        }

        if (matchData.matchCount() >= matchData.sessionData()->resultsPageSize()) {
            return;
        }

        if (!context.isValid()) {
            return;
        }

        if (service->noDisplay()) {
            continue;
        }

        const QString id = service->storageId();
        const QString name = service->desktopEntryName();
        const QString exec = service->exec();

        if (seen.contains(id) || seen.contains(exec)) {
            //qDebug() << "already seen" << id << exec;
            continue;
        }

        //qDebug() << "haven't seen" << id << "so processing now";
        seen.insert(id);
        seen.insert(exec);

        Sprinter::QueryMatch match;
        setupMatch(service, match, context);
        Sprinter::QuerySession::MatchPrecision precision = Sprinter::QuerySession::FuzzyMatch;

        // If the term was < 3 chars and NOT at the beginning of the App's name or Exec, then
        // chances are the user doesn't want that app.
        if (term.length() < 3) {
            if (name.startsWith(term) || exec.startsWith(term)) {
                precision = Sprinter::QuerySession::CloseMatch;
            } else {
                continue;
            }
        } else if (service->name().contains(term, Qt::CaseInsensitive)) {
            if (service->name().startsWith(term, Qt::CaseInsensitive)) {
                precision = Sprinter::QuerySession::CloseMatch;
            } else {
                precision = Sprinter::QuerySession::FuzzyMatch;
            }
        } else if (service->genericName().contains(term, Qt::CaseInsensitive)) {
            if (service->genericName().startsWith(term, Qt::CaseInsensitive)) {
                precision = Sprinter::QuerySession::CloseMatch;
            } else {
                precision = Sprinter::QuerySession::FuzzyMatch;
            }
        }

        //qDebug() << service->name() << "is this precise:" << precision;
        match.setPrecision(precision);
        matchData << match;
    }

    //search for applications whose categories contains the query
    query = QString("exist Exec and (exist Categories and '%1' ~subin Categories)").arg(term);
    services = KServiceTypeTrader::self()->query("Application", query);

    //qDebug() << service->name() << "is an exact match!" << service->storageId() << service->exec();
    foreach (const KService::Ptr &service, services) {
        if (skipCount < matchData.sessionData()->resultsOffset()) {
            ++skipCount;
            continue;
        }

        if (matchData.matchCount() >= matchData.sessionData()->resultsPageSize()) {
            return;
        }

        if (!context.isValid()) {
            return;
        }

        if (!service->noDisplay()) {
            const QString id = service->storageId();
            const QString exec = service->exec();
            if (seen.contains(id) || seen.contains(exec)) {
                //qDebug() << "already seen" << id << exec;
                continue;
            }

            Sprinter::QueryMatch match;
            setupMatch(service, match, context);
            match.setPrecision(Sprinter::QuerySession::FuzzyMatch);
            matchData << match;
        }
    }
}

bool ApplicationsRunner::exec(const Sprinter::QueryMatch &match)
{
    KService::Ptr service = KService::serviceByStorageId(match.data().toString());
    if (service) {
        return KRun::run(*service, QList<QUrl>(), 0);
    }

    return false;
}

void ApplicationsRunner::setupMatch(const KService::Ptr &service, Sprinter::QueryMatch &match, const Sprinter::QueryContext &context)
{
    const QString name = service->name();

    match.setTitle(name);
    match.setData(service->storageId());
    match.setType(Sprinter::QuerySession::ExecutableType);
    match.setSource(Sprinter::QuerySession::FromFilesystem);

    if (!service->genericName().isEmpty() && service->genericName() != name) {
        match.setText(service->genericName());
    } else if (!service->comment().isEmpty()) {
        match.setText(service->comment());
    }

    if (!service->icon().isEmpty()) {
        match.setImage(generateImage(QIcon::fromTheme(service->icon()), context));
    }
}

#include "moc_applications.cpp"

