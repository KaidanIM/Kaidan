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

#pragma once

#include <QAbstractListModel>
#include <QVector>

#include "PresenceCache.h"

class QXmppVersionIq;

class UserDevicesModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(QString jid READ jid WRITE setJid NOTIFY jidChanged)

public:
	enum Roles {
		Resource = Qt::UserRole + 1,
		Name,
		Version,
		OS
	};

	explicit UserDevicesModel(QObject *parent = nullptr);

	QHash<int, QByteArray> roleNames() const override;
	QVariant data(const QModelIndex &index, int role) const override;
	int rowCount(const QModelIndex &parent) const override;

	QString jid() const;
	void setJid(const QString &jid);

signals:
	void jidChanged();
	void clientVersionsRequested(const QString &bareJid, const QString &resource = {});

private slots:
	void handleClientVersionReceived(const QXmppVersionIq &versionIq);
	void handlePresenceChanged(PresenceCache::ChangeType type,
	                           const QString &jid,
	                           const QString &resource);
	void handlePresencesCleared();

private:
	struct DeviceInfo {
		DeviceInfo(const QString &resource);
		DeviceInfo(const QXmppVersionIq &);

		QString resource;
		QString name;
		QString version;
		QString os;
	};

	QString m_jid;
	QVector<DeviceInfo> m_devices;
};
