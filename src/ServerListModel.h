/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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

#ifndef SERVERLISTMODEL_H
#define SERVERLISTMODEL_H

#include <QAbstractListModel>
#include <QVector>

#include "ServerListItem.h"

class ServerListModel : public QAbstractListModel
{
	Q_OBJECT

public:
	enum Role {
		DisplayRole = Qt::DisplayRole,
		JidRole = Qt::UserRole + 1,
		SupportsInBandRegistrationRole,
		RegistrationWebPageRole,
		LanguageRole,
		CountryRole,
		FlagRole,
		IsCustomServerRole,
		WebsiteRole,
		OnlineSinceRole,
		HttpUploadSizeRole,
		MessageStorageDurationRole
	};
	Q_ENUM(Role)

	explicit ServerListModel(QObject *parent = nullptr);

	QHash<int, QByteArray> roleNames() const override;
	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

	// overloaded method for QML
	Q_INVOKABLE QVariant data(int row, ServerListModel::Role role) const;

	Q_INVOKABLE int randomlyChooseIndex() const;

private:
	void readItemsFromJsonFile(const QString &filePath);
	QVector<ServerListItem> serversSupportingInBandRegistration() const;
	QVector<ServerListItem> serversFromCountry(const QVector<ServerListItem> &preSelectedServers, const QString &country) const;
	int indexOfRandomlySelectedServer(const QVector<ServerListItem> &preSelectedServers) const;

	QVector<ServerListItem> m_items;
};

#endif // SERVERLISTMODEL_H
