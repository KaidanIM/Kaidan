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

#ifndef GLOBALS_H
#define GLOBALS_H

/**
 * Application information
 */
#define APPLICATION_ID "im.kaidan.kaidan"
#ifdef UBUNTU_TOUCH
#define APPLICATION_NAME "im.kaidan.kaidan"
#else
#define APPLICATION_NAME "kaidan"
#endif
#define APPLICATION_DISPLAY_NAME "Kaidan"
#define APPLICATION_DESCRIPTION "A simple, user-friendly Jabber/XMPP client"
#define VERSION_STRING "0.4.0-dev"

// Kaidan settings
#define KAIDAN_SETTINGS_AUTH_JID "auth/jid"
#define KAIDAN_SETTINGS_AUTH_RESOURCE "auth/resource"
#define KAIDAN_SETTINGS_AUTH_PASSWD "auth/password"

// XML namespaces
#define NS_SPOILERS "urn:xmpp:spoiler:0"

/**
 * Map of JIDs to contact names
 */
typedef QHash<QString, QString> ContactMap;

#endif // GLOBALS_H
