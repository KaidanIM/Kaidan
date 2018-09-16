/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2018 Kaidan developers and contributors
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
#include <QXmppClient.h>

namespace Enums {
	Q_NAMESPACE

	/**
	 * Enumeration of possible connection states.
	 */
	enum class ConnectionState : quint8 {
		StateDisconnected = QXmppClient::DisconnectedState,
		StateConnecting = QXmppClient::ConnectingState,
		StateConnected = QXmppClient::ConnectedState
	};
	Q_ENUM_NS(ConnectionState)

	/**
	 * Enumeration of possible disconnection reasons (compatible to gloox::
	 * ConnectionError)
	 */
	enum class DisconnectionReason : quint8 {
		ConnNoError,
		ConnAuthenticationFailed,
		ConnNotConnected,
		ConnTlsFailed,
		ConnTlsNotAvailable,
		ConnDnsError,
		ConnConnectionRefused,
		ConnNoSupportedAuth,
		ConnProxyUnreachable,
		ConnProxyAuthRequired,
		ConnProxyAuthFailed,
		ConnProxyNoSupportedAuth,
		ConnStreamError,
		ConnUserDisconnected,
	};
	Q_ENUM_NS(DisconnectionReason)

	// Alias, so that qDebug outputs the full name, but it can be
	// abrieviated in the code
	using DisconnReason = DisconnectionReason;

	/**
	 * Enumeration of different media/message types
	 */
	enum class MessageType : quint8 {
		MessageText,
		MessageFile,
		MessageImage,
		MessageVideo,
		MessageAudio,
		MessageDocument
	};
	Q_ENUM_NS(MessageType)
}

// Needed workaround to trigger older CMake auto moc versions to generate moc
// sources for this file (it only contains Q_NAMESPACE, which is new).
#if 0
Q_OBJECT
#endif

#endif // ENUMS_H
