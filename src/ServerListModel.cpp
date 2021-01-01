/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
 *  (see the LICENSE file for a full list of copyright authors)
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  In addition, as a special exception, the author of Kaidan gives
 *  permission to link the code of its release with the OpenSSL
 *  project's "OpenSSL" library (or with modified versions of it that
 *  use the same license as the "OpenSSL" library), and distribute the
 *  linked executables. You must obey the GNU General Public License in
 *  all respects for all of the code used other than "OpenSSL". If you
 *  modify this file, you may extend this exception to your version of
 *  the file, but you are not obligated to do so.  If you do not wish to
 *  do so, delete this exception statement from your version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ServerListModel.h"
// Qt
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QRandomGenerator>
// Kaidan
#include "ServerListItem.h"
#include "Globals.h"
#include "QmlUtils.h"

ServerListModel::ServerListModel(QObject *parent)
	: QAbstractListModel(parent)
{
	ServerListItem customServer(true);
	customServer.setJid(tr("Custom server"));
	m_items << customServer;

	readItemsFromJsonFile(SERVER_LIST_FILE_PATH);
}

QHash<int, QByteArray> ServerListModel::roleNames() const
{
	return {
		{DisplayRole, QByteArrayLiteral("display")},
		{JidRole, QByteArrayLiteral("jid")},
		{SupportsInBandRegistrationRole, QByteArrayLiteral("supportsInBandRegistration")},
		{RegistrationWebPageRole, QByteArrayLiteral("registrationWebPage")},
		{LanguageRole, QByteArrayLiteral("language")},
		{CountryRole, QByteArrayLiteral("country")},
		{FlagRole, QByteArrayLiteral("flag")},
		{IsCustomServerRole, QByteArrayLiteral("isCustomServer")},
		{WebsiteRole, QByteArrayLiteral("website")},
		{HttpUploadSizeRole, QByteArrayLiteral("httpUploadSize")},
		{MessageStorageDurationRole, QByteArrayLiteral("messageStorageDuration")}
	};
}

int ServerListModel::rowCount(const QModelIndex &parent) const
{
	// For list models, only the root node (an invalid parent) should return the list's size.
	// For all other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
	if (parent.isValid())
		return 0;

	return m_items.size();
}

QVariant ServerListModel::data(const QModelIndex &index, int role) const
{
	Q_ASSERT(checkIndex(index, QAbstractItemModel::CheckIndexOption::IndexIsValid | QAbstractItemModel::CheckIndexOption::ParentIsInvalid));

	const ServerListItem &item = m_items.at(index.row());

	switch (role) {
	case DisplayRole:
		return QStringLiteral("%1 %2").arg(item.flag(), item.jid());
	case JidRole:
		return item.jid();
	case SupportsInBandRegistrationRole:
		return item.supportsInBandRegistration();
	case RegistrationWebPageRole:
		return item.registrationWebPage();
	case LanguageRole:
		return item.language();
	case CountryRole:
		return item.country();
	case FlagRole:
		return item.flag();
	case IsCustomServerRole:
		return item.isCustomServer();
	case WebsiteRole:
		return QmlUtils::formatMessage(item.website().toString());
	case OnlineSinceRole:
		if (item.onlineSince() == -1)
			return QString();
		return QString::number(item.onlineSince());
	case HttpUploadSizeRole:
		switch (item.httpUploadSize()) {
		case -1:
			return QString();
		case 0:
			//: Unlimited file size for uploading files
			return tr("No limitation");
		default:
			return QLocale::system().formattedDataSize(item.httpUploadSize() * 1024LL * 1024LL, 0);
		}
	case MessageStorageDurationRole:
		switch (item.messageStorageDuration()) {
		case -1:
			return QString();
		case 0:
			//: Deletion of message history saved on server
			return tr("No limitation");
		default:
			return tr("%1 days").arg(item.messageStorageDuration());
		}
	}

	return {};
}

QVariant ServerListModel::data(int row, ServerListModel::Role role) const
{
	return data(index(row), role);
}

int ServerListModel::randomlyChooseIndex() const
{
	QVector<ServerListItem> serversWithInBandRegistration = serversSupportingInBandRegistration();

	QString systemCountryCode = QLocale::system().name().split(QStringLiteral("_")).last();
	QVector<ServerListItem> serversWithInBandRegistrationFromCountry = serversFromCountry(serversWithInBandRegistration, systemCountryCode);

	if (serversWithInBandRegistrationFromCountry.size() < SERVER_LIST_MIN_SERVERS_FROM_COUNTRY)
		return indexOfRandomlySelectedServer(serversWithInBandRegistration);

	return indexOfRandomlySelectedServer(serversWithInBandRegistrationFromCountry);
}

void ServerListModel::readItemsFromJsonFile(const QString &filePath)
{
	QFile file(filePath);
	if (!file.exists()) {
		qWarning() << "[ServerListModel] Could not parse server list:"
				   << filePath
				   << "- file does not exist!";
		return;
	}

	if (!file.open(QIODevice::ReadOnly)) {
		qWarning() << "[ServerListModel] Could not open file for reading:" << filePath;
		return;
	}

	QByteArray content = file.readAll();

	QJsonParseError parseError;
	QJsonArray jsonServerArray = QJsonDocument::fromJson(content, &parseError).array();
	if (jsonServerArray.isEmpty()) {
		qWarning() << "[ServerListModel] Could not parse server list JSON file or no servers defined.";
		qWarning() << "[ServerListModel] QJsonParseError:" << parseError.errorString() << "at" << parseError.offset;
		return;
	}

	for (auto jsonServerItem : jsonServerArray) {
		if (!jsonServerItem.isNull() && jsonServerItem.isObject())
			m_items << ServerListItem::fromJson(jsonServerItem.toObject());
	}

	// Sort the parsed servers.
	// The first item ("Custom server") is not sorted.
	if (m_items.size() > 1)
		std::sort(m_items.begin() + 1, m_items.end());
}

QVector<ServerListItem> ServerListModel::serversSupportingInBandRegistration() const
{
	QVector<ServerListItem> servers;

	// The search starts at index 1 to exclude the custom server.
	std::copy_if(m_items.begin() + 1, m_items.end(), std::back_inserter(servers), [](const ServerListItem &item) {
		return item.supportsInBandRegistration();
	});

	return servers;
}

QVector<ServerListItem> ServerListModel::serversFromCountry(const QVector<ServerListItem> &preSelectedServers, const QString &country) const
{
	QVector<ServerListItem> servers;

	for (const auto &server : preSelectedServers) {
		if (server.country() == country)
			servers << server;
	}

	return servers;
}

int ServerListModel::indexOfRandomlySelectedServer(const QVector<ServerListItem> &preSelectedServers) const
{
	return m_items.indexOf(preSelectedServers.at(QRandomGenerator::global()->generate() % preSelectedServers.size()));
}
