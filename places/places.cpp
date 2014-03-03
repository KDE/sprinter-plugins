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

#include "places.h"

#include <KIOWidgets/KRun>
#include <KI18n/KLocalizedString>

PlacesSessionData::PlacesSessionData(Sprinter::Runner *runner)
    : RunnerSessionData(runner),
      m_places()
{
    connect(runner, SIGNAL(startQuery(QString,Sprinter::QueryContext)),
            this, SLOT(startQuery(QString, Sprinter::QueryContext)));
    connect(runner, SIGNAL(startExec(Sprinter::QueryMatch)),
            this, SLOT(startExec(Sprinter::QueryMatch)));

    connect(&m_places, &KFilePlacesModel::setupDone, [this](QModelIndex index, bool success) {
            if (success) {
                new KRun(m_places.url(index), nullptr);
            }
        });
}

void PlacesSessionData::startQuery(const QString &query, const Sprinter::QueryContext &context)
{
    if (!context.isValid()) {
        return;
    }

    // Show all places if query is empty
    const bool all = query.isEmpty();

    QVector<Sprinter::QueryMatch> matches;
    for (int i = 0; i <= m_places.rowCount(); ++i) {
        const QModelIndex index = m_places.index(i, 0);

        const QString text = m_places.text(index);
        if (text.isEmpty()) {
            continue;
        }

        Sprinter::QuerySession::MatchPrecision precision = Sprinter::QuerySession::UnrelatedMatch;
        if (all || text.compare(query, Qt::CaseInsensitive) == 0) {
            precision = Sprinter::QuerySession::ExactMatch;
        } else if (text.startsWith(query, Qt::CaseInsensitive)) {
            precision = Sprinter::QuerySession::CloseMatch;
        } else if (text.contains(query, Qt::CaseInsensitive)) {
            precision = Sprinter::QuerySession::FuzzyMatch;
        }

        if (precision != Sprinter::QuerySession::UnrelatedMatch) {
            Sprinter::QueryMatch match;
            match.setType(Sprinter::QuerySession::FilesystemLocationType);
            match.setSource(Sprinter::QuerySession::FromFilesystem);
            match.setPrecision(precision);
            match.setImage(runner()->generateImage(m_places.icon(index), context));
            match.setText(text);

            // if we have to mount it set the device udi instead of the URL, as we can't open it directly
            if (m_places.isDevice(index) && m_places.setupNeeded(index)) {
                match.setData(m_places.deviceForIndex(index).udi());
            } else {
                match.setData(m_places.url(index));
            }

            matches << match;
        }
    }

    setMatches(matches, context);
}

void PlacesSessionData::startExec(const Sprinter::QueryMatch& match)
{
    if (match.data().canConvert<QUrl>()) {
        new KRun(match.data().toUrl(), nullptr);
    } else if (match.data().canConvert<QString>()) {
        // Search our list for the device with the same udi, then set it up (mount it).
        const QString deviceUdi = match.data().toString();

        for (int i = 0; i <= m_places.rowCount(); ++i) {
            const QModelIndex index = m_places.index(i, 0);

            if (m_places.isDevice(index) && m_places.deviceForIndex(index).udi() == deviceUdi) {
                m_places.requestSetup(index);
            }
        }
    }
}

PlacesRunner::PlacesRunner(QObject *parent)
    : Runner(parent),
      m_placesWord(i18n("places"))
{
    setMinQueryLength(m_placesWord.length());
}

Sprinter::RunnerSessionData *PlacesRunner::createSessionData()
{
    return new PlacesSessionData(this);
}

void PlacesRunner::match(Sprinter::MatchData &matchData)
{
    const QString term = matchData.queryContext().query();

    if (!dynamic_cast<PlacesSessionData *>(matchData.sessionData())) {
        return;
    }

    if (term.startsWith(m_placesWord, Qt::CaseInsensitive)) {
        matchData.setAsynchronous(true);
        const QString query = term.right(term.length() - m_placesWord.length()).trimmed();
        emit startQuery(query, matchData.queryContext());
    }
}

bool PlacesRunner::exec(const Sprinter::QueryMatch &match)
{
    if (!dynamic_cast<PlacesSessionData *>(match.sessionData())) {
        return false;
    }

    emit startExec(match);
    return true;
}

#include "places.moc"