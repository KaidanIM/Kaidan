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

#ifndef ENUMS_H
#define ENUMS_H

#include <QtGlobal>
#include <QObject>
#include <QMetaEnum>
#include <QXmppClient.h>

#define ENABLE_IF(...) typename std::enable_if<__VA_ARGS__>::type* = nullptr

template <typename... Ts> struct make_void { typedef void type; };
template <typename... Ts> using void_t = typename make_void<Ts...>::type;

// primary template handles types that have no nested ::enum_type member, like standard enum
template <typename, typename = void_t<>>
struct has_enum_type : std::false_type { };

// specialization recognizes types that do have a nested ::enum_type member, like QFlags enum
template <typename T>
struct has_enum_type<T, void_t<typename T::enum_type>> : std::true_type { };

namespace Enums {
	Q_NAMESPACE

	/**
	 * Enumeration of possible connection states.
	 */
	enum class ConnectionState {
		StateDisconnected = QXmppClient::DisconnectedState,
		StateConnecting = QXmppClient::ConnectingState,
		StateConnected = QXmppClient::ConnectedState
	};
	Q_ENUM_NS(ConnectionState)

	/**
	 * Enumeration of different media/message types
	 */
	enum class MessageType {
		MessageUnknown = -1,
		MessageText,
		MessageFile,
		MessageImage,
		MessageVideo,
		MessageAudio,
		MessageDocument,
		MessageGeoLocation
	};
	Q_ENUM_NS(MessageType)

	/**
	 * Enumeration of contact availability states
	 */
	enum class AvailabilityTypes {
		PresError,
		PresUnavailable,
		PresOnline,
		PresAway,
		PresXA,
		PresDND,
		PresChat,
		PresInvisible
	};
	Q_ENUM_NS(AvailabilityTypes)

	/**
	 * Enumeration of different message delivery states
	 */
	enum class DeliveryState {
		Pending,
		Sent,
		Delivered,
		Error
	};
	Q_ENUM_NS(DeliveryState)

	/**
	 * State which specifies how the XMPP login URI was used
	 */
	enum class LoginByUriState {
		Connecting,         ///< The JID and password are included in the URI and the client is connecting.
		PasswordNeeded,     ///< The JID is included in the URI but not the password.
		InvalidLoginUri     ///< The URI cannot be used to log in.
	};
	Q_ENUM_NS(LoginByUriState)

	template <typename T, ENABLE_IF(!has_enum_type<T>::value && std::is_enum<T>::value)>
	QString toString(const T flag) {
		static const QMetaEnum e = QMetaEnum::fromType<T>();
		return QString::fromLatin1(e.valueToKey(static_cast<int>(flag)));
	}

	template <typename T, ENABLE_IF(has_enum_type<T>::value)>
	QString toString(const T flags) {
		static const QMetaEnum e = QMetaEnum::fromType<T>();
		return QString::fromLatin1(e.valueToKeys(static_cast<int>(flags)));
	}
}

// Needed workaround to trigger older CMake auto moc versions to generate moc
// sources for this file (it only contains Q_NAMESPACE, which is new).
#if 0
Q_OBJECT
#endif

#endif // ENUMS_H
