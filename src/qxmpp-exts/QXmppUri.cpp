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

#include "QXmppUri.h"

#include <QUrlQuery>

constexpr QStringView SCHEME = u"xmpp";
constexpr QStringView PREFIX = u"xmpp:";
constexpr QChar QUERY_ITEM_DELIMITER = u';';
constexpr QChar QUERY_ITEM_KEY_DELIMITER = u'=';

// Query types representing actions, e.g. "join" in
// "xmpp:group@example.org?join" for joining a group chat
constexpr std::array<QStringView, 17> QUERY_TYPES = {
	QStringView(),
	u"command",
	u"disco",
	u"invite",
	u"join",
	u"login",
	u"message",
	u"pubsub",
	u"recvfile",
	u"register",
	u"remove",
	u"roster",
	u"sendfile",
	u"subscribe",
	u"unregister",
	u"unsubscribe",
	u"vcard"
};

// QXmppMessage types as strings
constexpr std::array<QStringView, 5> MESSAGE_TYPES = {
	u"error",
	u"normal",
	u"chat",
	u"groupchat",
	u"headline"
};

///
/// Parses the URI from a string.
///
/// @param input string which may present an XMPP URI
///
QXmppUri::QXmppUri(QString input)
{
	QUrl url(input);
	if (!url.isValid() || url.scheme() != SCHEME)
		return;

	m_jid = url.path();

	if (!url.hasQuery())
		return;

	QUrlQuery query;
	query.setQueryDelimiters(QUERY_ITEM_KEY_DELIMITER, QUERY_ITEM_DELIMITER);
	query.setQuery(url.query(QUrl::FullyEncoded));

	if (!setAction(query))
		return;

	setQueryKeyValuePairs(query);
}

///
/// Decodes this URI to a string.
///
/// @return this URI as a string
///
QString QXmppUri::toString()
{
	QUrl url;
	url.setScheme(SCHEME.toString());
	url.setPath(m_jid);

	QUrlQuery query;
	query.setQueryDelimiters(QUERY_ITEM_KEY_DELIMITER, QUERY_ITEM_DELIMITER);

	// Add the query item.
	if (m_action != None) {
		addItemsToQuery(query);
	}

	url.setQuery(query);

	return url.toEncoded(QUrl::FullyEncoded);
}

///
/// Returns the JID this URI is about.
///
/// This can also be e.g. a MUC room in case of a Join action.
///
QString QXmppUri::jid() const
{
	return m_jid;
}

///
/// Sets the JID this URI links to.
///
/// @param jid JID to be set
///
void QXmppUri::setJid(const QString &jid)
{
	m_jid = jid;
}

///
/// Returns the action of this URI.
///
/// This is None in case no action is included.
///
QXmppUri::Action QXmppUri::action() const
{
	return m_action;
}

///
/// Sets the action of this URI, e.g. Join for a URI with the query type
/// \c ?join".
///
/// @param action action to be set
///
void QXmppUri::setAction(const Action &action)
{
	m_action = action;
}

///
/// Returns the password of a login action
///
QString QXmppUri::password() const
{
	return m_password;
}

///
/// Sets the password of a login action.
///
/// @param password
///
void QXmppUri::setPassword(const QString &password)
{
	m_password = password;
}

///
/// In case the URI has a message query, this can be used to get the attached
/// message content directly as \c QXmppMessage.
///
QXmppMessage QXmppUri::message() const
{
	return m_message;
}

///
/// Sets the attached message for a Message action.
///
/// Supported properties are: body, from, id, thread, type, subject.
/// If you want to include the message type, ensure that \c hasMessageType is
/// set to true.
///
/// @param message message to be set
///
void QXmppUri::setMessage(const QXmppMessage &message)
{
	m_action = Message;
	m_message = message;
}

///
/// Returns true, if the attached message's type is included.
///
/// This is required because \c QXmppMessage has no option to set no type.
///
bool QXmppUri::hasMessageType() const
{
	return m_hasMessageType;
}

///
/// Sets whether to include the message's type.
///
/// This is required because \c QXmppMessage has no option to set an empty
/// type.
///
/// @param hasMessageType true if the message's type should be included
///
void QXmppUri::setHasMessageType(bool hasMessageType)
{
	m_hasMessageType = hasMessageType;
}

///
/// Checks whether the string starts with the XMPP scheme.
///
/// @param uri URI to check for XMPP scheme
///
bool QXmppUri::isXmppUri(const QString &uri)
{
	return uri.startsWith(PREFIX);
}

///
/// Sets the action represented by its query type.
///
/// The query item used as the query type is retrieved. That query item does
/// not consist of a key-value pair because the query type is only a string.
/// Therefore, only the first item of the first retrieved pair is needed.
///
/// \param query query from which its type is used to create a corresponding
/// action
///
/// \return true if the action could be set or false if the action could not be
/// set because no corresponding query type could be found
///
bool QXmppUri::setAction(const QUrlQuery &query)
{
	// Check if there is at least one query item.
	if (query.isEmpty())
		return false;

	auto queryItems = query.queryItems();
	auto firstQueryItem = queryItems.first();

	const auto queryTypeIndex = std::find(QUERY_TYPES.cbegin(), QUERY_TYPES.cend(), firstQueryItem.first);

	// Check if the first query item is a valid action (i.e. a query item
	// with a query type as its key and without a value).
	if (queryTypeIndex == QUERY_TYPES.cend() || !firstQueryItem.second.isEmpty())
		return false;

	m_action = Action(std::distance(MESSAGE_TYPES.cbegin(), queryTypeIndex));
	return true;
}

///
/// Extracts the query type which represents the URI's action.
///
/// \return the query type for the URI's action
///
QString QXmppUri::queryType() const
{
	return QUERY_TYPES[int(m_action)].toString();
}

///
/// Sets the key-value pairs of a query.
///
/// \param query query which contains key-value pairs
///
void QXmppUri::setQueryKeyValuePairs(const QUrlQuery &query)
{
	switch (m_action) {
	case Message:
		m_message.setSubject(queryItemValue(query, "subject"));
		m_message.setBody(queryItemValue(query, "body"));
		m_message.setThread(queryItemValue(query, "thread"));
		m_message.setId(queryItemValue(query, "id"));
		m_message.setFrom(queryItemValue(query, "from"));
		if (!queryItemValue(query, "type").isEmpty()) {
			const auto itr = std::find(MESSAGE_TYPES.cbegin(), MESSAGE_TYPES.cend(), queryItemValue(query, "type"));
			if (itr != MESSAGE_TYPES.cend())
				m_message.setType(QXmppMessage::Type(std::distance(MESSAGE_TYPES.cbegin(), itr)));
		} else {
			m_hasMessageType = false;
		}
		break;
	case Login:
		m_password = queryItemValue(query, "password");
		break;
	default:
		break;
	}
}

///
/// Adds all query items of this URI to a query.
///
/// \param query query to which the items are added
///
void QXmppUri::addItemsToQuery(QUrlQuery &query) const
{
	// Add the query type for the corresponding action.
	query.addQueryItem(queryType(), {});

	// Add all remaining query items that are the key-value pairs.
	switch (m_action) {
	case Message:
		addKeyValuePairToQuery(query, "from", m_message.from());
		addKeyValuePairToQuery(query, "id", m_message.id());
		if (m_hasMessageType)
			addKeyValuePairToQuery(query, "type", MESSAGE_TYPES[int(m_message.type())]);
		addKeyValuePairToQuery(query, "subject", m_message.subject());
		addKeyValuePairToQuery(query, "body", m_message.body());
		addKeyValuePairToQuery(query, "thread", m_message.thread());
		break;
	case Login:
		addKeyValuePairToQuery(query, "password", m_password);
		break;
	default:
		break;
	}
}

///
/// Adds a key-value pair to a query if the value is not empty.
///
/// @param query to which the key-value pair is added
/// @param key key of the value
/// @param value value of the key
///
void QXmppUri::addKeyValuePairToQuery(QUrlQuery &query, const QString &key, QStringView value)
{
	if (!value.isEmpty())
		query.addQueryItem(key, value.toString());
}

///
/// Extracts the fully-encoded value of a query's key-value pair.
///
/// @param query query containing the key-value pair
/// @param key of the searched value
///
/// @return the value of the key
///
QString QXmppUri::queryItemValue(const QUrlQuery &query, const QString &key)
{
	return query.queryItemValue(key, QUrl::FullyDecoded);
}
