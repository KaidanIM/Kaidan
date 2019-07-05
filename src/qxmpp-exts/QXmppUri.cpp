/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

#include "QXmppUri.h"
#include <QUrlQuery>

/// actions, e.g. "join" in "xmpp:group@example.org?join" for joining a group chat

const QStringList ACTION_STRINGS = QStringList()
        << ""
        << "command"
        << "disco"
        << "invite"
        << "join"
        << "login"
        << "message"
        << "pubsub"
        << "recvfile"
        << "register"
        << "remove"
        << "roster"
        << "sendfile"
        << "subscribe"
        << "unregister"
        << "unsubscribe"
        << "vcard";

/// types of a "message" action

const QStringList MESSAGE_TYPE_STRINGS = QStringList()
        << "error"
        << "normal"
        << "chat"
        << "groupchat"
        << "headline";

/// keys of a message action's key-value pair

const QStringList MESSAGE_ATTRIBUTES = QStringList()
        << "subject"
        << "body"
        << "thread"
        << "from"
        << "id"
        << "type";

/// Adds a key-value pair to the given pairs if val is not empty.
/// @param pairs which will be extended by the key-value pair with the given key
/// @param key key of the given value
/// @param val value of given key

void helperToAddPair(QList<QPair<QString, QString>> &pairs, const QString &key,
                     const QString &val)
{
	if (!val.isEmpty())
		pairs << qMakePair(key, val);
}

/// @return value of a given key in a given query
/// @param query query to search for the key
/// @param key to get the corresponding value

QString helperToGetQueryItemValue(const QUrlQuery &query, const QString &key)
{
	return query.queryItemValue(key, QUrl::FullyDecoded);
}

/// Parses the URI from a string.
/// @param input string which may present an XMPP URI

QXmppUri::QXmppUri(QString input)
{
	// We're replacing ';' with '&', so we can reuse the QUrlQuery.
	QUrl url(input.replace(";", "&"));
	if (!url.isValid() || url.scheme() != "xmpp")
		return;

	// set JID
	setJid(url.path());

	if (!input.contains("?"))
		return;

	QUrlQuery query(url.query());
	// check that there are query items (key-value pairs)
	if (!query.queryItems().size())
		return;

	m_action = static_cast<Action>(
	        ACTION_STRINGS.indexOf(query.queryItems().first().first)
	);

	switch (m_action) {
	case Message:
		m_message.setSubject(helperToGetQueryItemValue(query, "subject"));
		m_message.setBody(helperToGetQueryItemValue(query, "body"));
		m_message.setThread(helperToGetQueryItemValue(query, "thread"));
		m_message.setId(helperToGetQueryItemValue(query, "id"));
		m_message.setFrom(helperToGetQueryItemValue(query, "from"));
		if (!helperToGetQueryItemValue(query, "type").isEmpty())
			m_message.setType(static_cast<QXmppMessage::Type>(
			        MESSAGE_TYPE_STRINGS.indexOf(helperToGetQueryItemValue(query, "type"))
			));
		else
			m_hasMessageType = false;
		break;
	case Login:
		m_password = helperToGetQueryItemValue(query, "password");
		break;
	default:
		break;
	}
}

/// Decodes the URI to a string.
///
/// \returns Full XMPP URI

QString QXmppUri::toString()
{
	QUrl url;
	url.setScheme("xmpp");
	url.setPath(m_jid);

	// Create query items (parameters)
	QUrlQuery query;
	QList<QPair<QString, QString>> queryItems;

	switch (m_action) {
	case Message:
        helperToAddPair(queryItems, "body", m_message.body());
        helperToAddPair(queryItems, "from", m_message.from());
        helperToAddPair(queryItems, "id", m_message.id());
        helperToAddPair(queryItems, "thread", m_message.thread());
        if (m_hasMessageType)
            helperToAddPair(queryItems, "type", MESSAGE_TYPE_STRINGS.at(
                    int(m_message.type())));
        helperToAddPair(queryItems, "subject", m_message.subject());
        break;
    case Login:
        helperToAddPair(queryItems, "password", m_password);
        break;
	default:
        break;
	}
	query.setQueryItems(queryItems);

	QString output = url.toEncoded();
	if (m_action != None) {
	// add action
	output += "?";
	output += ACTION_STRINGS.at(int(m_action));

	// add parameters
	QString queryStr = QUrl::toPercentEncoding(query.toString().replace("&", ";"), ";=");
	if (!query.isEmpty()) {
		output += ";";
		output += queryStr;
	}
	}

	return output;
}

/// Returns the JID this URI is about.
///
/// This can also be e.g. a MUC room in case of a Join action.

QString QXmppUri::jid() const
{
	return m_jid;
}

/// Sets the JID this XMPP URI links to.
///
/// @param jid JID to be set

void QXmppUri::setJid(const QString &jid)
{
	m_jid = jid;
}

/// Returns the action of this XMPP URI.
///
/// This is None in case no action is included.

QXmppUri::Action QXmppUri::action() const
{
	return m_action;
}

/// Sets the action of this XMPP URI, e.g. Join for a URI ending with
/// \c ?join".
///
/// @param action action to be set

void QXmppUri::setAction(const Action &action)
{
	m_action = action;
}

/// Returns true if this XMPP URI has the given action.
///
/// @param action action to be checked for

bool QXmppUri::hasAction(const Action &action)
{
	return m_action == action;
}

QString QXmppUri::password() const
{
	return m_password;
}

/// In case the URI has a message query, this can be used to get the attached
/// message content directly as \c QXmppMessage.

QXmppMessage QXmppUri::message() const
{
	return m_message;
}

/// Sets the attached message for a Message action.
///
/// Supported properties are: body, from, id, thread, type, subject.
/// If you want to include the message type, ensure that \c hasMessageType is
/// set to true.
///
/// @param message message to be set

void QXmppUri::setMessage(const QXmppMessage &message)
{
	setAction(Message);
	m_message = message;
}

/// Returns true, if the attached message's type is included.
///
/// This is required because \c QXmppMessage has no option to set no type.

bool QXmppUri::hasMessageType() const
{
	return m_hasMessageType;
}

/// Sets whether to include the message's type.
///
/// This is required because \c QXmppMessage has no option to set an empty type.
///
/// @param hasMessageType true if the message's type should be included

void QXmppUri::setHasMessageType(bool hasMessageType)
{
	m_hasMessageType = hasMessageType;
}

/// Checks whether the string starts with the XMPP scheme.
///
/// @param uri URI to check for XMPP scheme

bool QXmppUri::isXmppUri(const QString &uri)
{
	return uri.startsWith("xmpp:");
}
