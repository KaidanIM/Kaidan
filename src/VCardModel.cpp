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

#include "VCardModel.h"

#include "Kaidan.h"
#include "VCardManager.h"

VCardModel::VCardModel(QObject *parent)
	: QAbstractListModel(parent)
{
	connect(
		Kaidan::instance()->getClient()->getVCardManager(),
		&VCardManager::vCardReceived,
		this,
		&VCardModel::handleVCardReceived
	);
}

QHash<int, QByteArray> VCardModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Key] = "key";
	roles[Value] = "value";
	return roles;
}

int VCardModel::rowCount(const QModelIndex &parent) const
{
	// For list models only the root node (an invalid parent) should return the
	// list's size. For all other (valid) parents, rowCount() should return 0 so
	// that it does not become a tree model.
	if (parent.isValid())
		return 0;

	return m_vCard.size();
}

QVariant VCardModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid())
		return {};

	switch(role) {
	case Key:
		return m_vCard.at(index.row()).key();
	case Value:
		return m_vCard.at(index.row()).value();
	}
	return {};
}

void VCardModel::handleVCardReceived(const QXmppVCardIq &vCard)
{
	if (vCard.from() == m_jid) {
		beginResetModel();

		m_vCard.clear();

		if (!vCard.fullName().isEmpty())
			m_vCard << Item(tr("Name"), vCard.fullName());

		if (!vCard.nickName().isEmpty())
			m_vCard << Item(tr("Nickname"), vCard.nickName());

		if (!vCard.description().isEmpty())
			m_vCard << Item(tr("About"), vCard.description());

		if (!vCard.email().isEmpty())
			m_vCard << Item(tr("Email"), vCard.email());

		if (!vCard.birthday().isNull() && vCard.birthday().isValid())
			m_vCard << Item(tr("Birthday"), vCard.birthday().toString());

		if (!vCard.url().isEmpty())
			m_vCard << Item(tr("Website"), vCard.url());

		endResetModel();
	}
}

QString VCardModel::jid() const
{
	return m_jid;
}

void VCardModel::setJid(const QString &jid)
{
	m_jid = jid;
	emit jidChanged();

	Kaidan::instance()->vCardRequested(jid);
}

VCardModel::Item::Item(const QString &key, const QString &value)
	: m_key(key), m_value(value)
{
}

QString VCardModel::Item::key() const
{
	return m_key;
}

void VCardModel::Item::setKey(const QString &key)
{
	m_key = key;
}

QString VCardModel::Item::value() const
{
	return m_value;
}

void VCardModel::Item::setValue(const QString &value)
{
	m_value = value;
}
