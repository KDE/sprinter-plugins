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

#include "uri.h"

#include <QDebug>
#include <QEventLoop>
#include <QIcon>

#include <KIOCore/KProtocolInfo>
#include <KIOWidgets/KRun>
#include <KIOWidgets/KUriFilter>

UriRunner::UriRunner(QObject *parent)
    : Sprinter::Runner(parent)
{
     m_filterList << QStringLiteral("kshorturifilter");
}

void UriRunner::match(Sprinter::MatchData &matchData)
{
    QString location = matchData.queryContext().query();
    KUriFilter *filter = KUriFilter::self();
    const bool filtered = filter->filterUri(location, m_filterList);

    if (!filtered) {
        return;
    }

    qDebug() << "filtered to" << location;
    QUrl url(location);

    if (!KProtocolInfo::isKnownProtocol(url.scheme())) {
        return;
    }

    Sprinter::QueryMatch match;
    match.setData(location);
    QIcon icon = QIcon::fromTheme(KProtocolInfo::icon(url.scheme()));
    if (!icon.isNull()) {
        match.setImage(icon.pixmap(matchData.queryContext().imageSize()).toImage());
    }

    if (KProtocolInfo::isHelperProtocol(url.scheme())) {
        //qDebug() << "helper protocol" << url.scheme() <<"call external application" ;
        if (url.scheme() == "mailto") {
            match.setTitle(tr("Send email to %1").arg(url.path()));
            match.setType(Sprinter::QuerySession::MessageType);
        } else {
            match.setTitle(tr("Launch with %1").arg( KProtocolInfo::exec(url.scheme())));
            match.setType(Sprinter::QuerySession::NetworkLocationType);
        }
    } else {
        //kDebug() << "protocol managed by browser" << url.scheme();
        match.setTitle(tr("Go to %1").arg(url.toString()));
        match.setType(Sprinter::QuerySession::NetworkLocationType);
    }

    match.setSource(Sprinter::QuerySession::FromInternalSource);
    match.setPrecision(Sprinter::QuerySession::ExactMatch);
    matchData << match;
}

bool UriRunner::exec(const Sprinter::QueryMatch &match)
{
    bool success = false;
    QEventLoop loop;
    KRun *krun = new KRun(match.data().toString(), 0);
    connect(krun, &KRun::finished,
            [&]() { success = !krun->hasError(); loop.exit(); });
    loop.exec();
    return success;
}

#include "moc_uri.cpp"
