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

#ifndef GLOBALS_H
#define GLOBALS_H

#include <QLatin1String>

// Kaidan settings
#define KAIDAN_SETTINGS_AUTH_ONLINE "auth/online"
#define KAIDAN_SETTINGS_AUTH_JID "auth/jid"
#define KAIDAN_SETTINGS_AUTH_JID_RESOURCE_PREFIX "auth/jidResourcePrefix"
#define KAIDAN_SETTINGS_AUTH_PASSWD "auth/password"
#define KAIDAN_SETTINGS_AUTH_PASSWD_VISIBILITY "auth/passwordVisibility"
#define KAIDAN_SETTINGS_NOTIFICATIONS_MUTED "muted/"

#define KAIDAN_JID_RESOURCE_DEFAULT_PREFIX APPLICATION_DISPLAY_NAME

#define INVITATION_URL "https://i.kaidan.im/#"
#define APPLICATION_SOURCE_CODE_URL "https://invent.kde.org/network/kaidan"
#define ISSUE_TRACKING_URL "https://invent.kde.org/network/kaidan/issues"

constexpr auto MESSAGE_MAX_CHARS = 1e4;

// XML namespaces
#define NS_CARBONS "urn:xmpp:carbons:2"

// SQL
#define DB_CONNECTION "kaidan-messages"
#define DB_FILENAME "messages.sqlite3"
#define DB_MSG_QUERY_LIMIT 20
#define DB_TABLE_INFO "dbinfo"
#define DB_TABLE_ROSTER "Roster"
#define DB_TABLE_MESSAGES "Messages"

//
// Credential generation
//

// Length of generated usernames
constexpr auto GENERATED_USERNAME_LENGTH = 6;

// Lower bound of the length for generated passwords (inclusive)
constexpr auto GENERATED_PASSWORD_LENGTH_LOWER_BOUND = 20;

// Upper bound of the length for generated passwords (inclusive)
constexpr auto GENERATED_PASSWORD_LENGTH_UPPER_BOUND = 30;

// Characters used for password generation
#define GENERATED_PASSWORD_ALPHABET QLatin1String( \
	"abcdefghijklmnopqrstuvwxyz" \
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
	"`1234567890-=" \
	"~!@#$%^&*()_+" \
	"[];'\\,./{}:\"|<>?" \
)

// Number of characters used for password generation
#define GENERATED_PASSWORD_ALPHABET_LENGTH GENERATED_PASSWORD_ALPHABET.size()

// QXmpp version compatibility
constexpr auto QXMPP_REQUIRED_VERSION_FOR_REGISTRATION = QT_VERSION_CHECK(1, 2, 0);

/**
 * Path of the JSON server list file
 */
#define SERVER_LIST_FILE_PATH QStringLiteral(":/data/servers.json")

/**
 * Number of servers required in a country so that only servers from that country are
 * randomly selected.
 */
constexpr auto SERVER_LIST_MIN_SERVERS_FROM_COUNTRY = 2;

/**
 * Name of the @c QQuickImageProvider for Bits of Binary.
 */
#define BITS_OF_BINARY_IMAGE_PROVIDER_NAME "bits-of-binary"

#endif // GLOBALS_H
