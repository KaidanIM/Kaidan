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

#ifndef QXMPPURI_H
#define QXMPPURI_H

#include <QUrlQuery>

#include <QXmppMessage.h>

///
/// This class represents an XMPP URI as specified by RFC 5122 -
/// Internationalized Resource Identifiers (IRIs) and Uniform Resource
/// Identifiers (URIs) for the Extensible Messaging and Presence Protocol
/// (XMPP) and XEP-0147: XMPP URI Scheme Query Components.
///
/// A QUrlQuery is used by this class to represent a query (component) of an
/// XMPP URI. A query conisists of query items which can be the query type or a
/// key-value pair.
///
/// A query type is used to perform an action while the key-value pairs are
/// used to define its behavior.
///
/// Example:
/// xmpp:alice@example.org?message;subject=Hello
///
/// query (component): message;subject=Hello;body=world
/// query items: message, subject=Hello, body=world
/// query type: message
/// key-value pair 1: subject=Hello
/// key-value pair 2: body=world
///
class QXmppUri
{
public:
	enum Action {
		None,
		Command,
		Disco,
		Invite,
		Join,
		Login,
		Message,
		PubSub,
		RecvFile,
		Register,
		Remove,
		Roster,
		SendFile,
		Subscribe,
		Unregister,
		Unsubscribe,
		VCard,
	};

	QXmppUri() = default;
	QXmppUri(QString uri);

	QString toString();

	QString jid() const;
	void setJid(const QString &jid);

	Action action() const;
	void setAction(const Action &action);

	// login
	QString password() const;
	void setPassword(const QString &password);

	// message
	QXmppMessage message() const;
	void setMessage(const QXmppMessage&);

	bool hasMessageType() const;
	void setHasMessageType(bool hasMessageType);

	static bool isXmppUri(const QString &uri);

private:
	bool setAction(const QUrlQuery &query);
	QString queryType() const;
	void setQueryKeyValuePairs(const QUrlQuery &query);
	void addItemsToQuery(QUrlQuery &query) const;

	static void addKeyValuePairToQuery(QUrlQuery &query, const QString &key, QStringView val);
	static QString queryItemValue(const QUrlQuery &query, const QString &key);

	QString m_jid;
	Action m_action = None;

	// login
	QString m_password;

	// message
	QXmppMessage m_message;
	bool m_hasMessageType = false;
};

#endif // QXMPPURI_H
