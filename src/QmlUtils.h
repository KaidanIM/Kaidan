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

#ifndef QMLUTILS_H
#define QMLUTILS_H

#include <QObject>
#include <QUrl>

#include "Globals.h"
#include "Enums.h"

using namespace Enums;

/**
 * @brief C++ utitlities to be used in QML
 *
 * The methods are not static, because they need to be used from QML and registered in
 * Qt.
 */
class QmlUtils : public QObject
{
	Q_OBJECT

public:
	static QmlUtils *instance();

	QmlUtils(QObject *parent = nullptr);
	~QmlUtils();

	/**
	 * Returns the icon belonging to the given presence status type
	 */
	Q_INVOKABLE static QString presenceTypeToIcon(Enums::AvailabilityTypes type);

	/**
	 * Returns the text belonging to the given presence status type
	 */
	Q_INVOKABLE static QString presenceTypeToText(Enums::AvailabilityTypes type);

	/**
	 * Returns the color belonging to the given presence status type
	 */
	Q_INVOKABLE static QColor presenceTypeToColor(Enums::AvailabilityTypes type);

	/**
	 * Returns a URL to a given resource file name
	 *
	 * This will check various paths which could contain the searched file.
	 * If the file was found, it'll return a `file://` or a `qrc:/` url to
	 * the file.
	 */
	Q_INVOKABLE static QString getResourcePath(const QString &resourceName);

	/**
	 * Returns a string of this build's Kaidan version
	 */
	Q_INVOKABLE static QString versionString()
	{
		return QStringLiteral(VERSION_STRING);
	}

	/**
	 * Returns the name of this application as it should be displayed to the user
	 */
	Q_INVOKABLE static QString applicationDisplayName()
	{
		return QStringLiteral(APPLICATION_DISPLAY_NAME);
	}

	/**
	 * Returns the URL where the source code of this application can be found
	 */
	Q_INVOKABLE static QUrl applicationSourceCodeUrl()
	{
		return {QStringLiteral(APPLICATION_SOURCE_CODE_URL)};
	}

	/**
	 * Returns an invitation URL to the given JID
	 */
	Q_INVOKABLE static QUrl invitationUrl(const QString &jid)
	{
		return {QStringLiteral(INVITATION_URL) + jid};
	}

	/**
	 * Returns a string without new lines, unneeded spaces, etc.
	 *
	 * See QString::simplified for more information.
	 */
	Q_INVOKABLE static QString removeNewLinesFromString(const QString &input)
	{
		return input.simplified();
	}

	/**
	 * Checks whether a file is an image and could be displayed as such.
	 * @param fileUrl URL to the possible image file
	 */
	Q_INVOKABLE static bool isImageFile(const QUrl &fileUrl);

	/**
	 * Copy text to the clipboard
	 */
	Q_INVOKABLE static void copyToClipboard(const QString &text);

	/**
	 * Returns the file name from a URL
	 */
	Q_INVOKABLE static QString fileNameFromUrl(const QUrl &url);

	/**
	 * Returns the file size from a URL
	 */
	Q_INVOKABLE static QString fileSizeFromUrl(const QUrl &url);

	/**
	 * Styles/formats a message for displaying
	 *
	 * This currently only adds some link highlighting
	 */
	Q_INVOKABLE static QString formatMessage(const QString &message);

	/**
	 * Returns a consistent user color generated from the nickname.
	 */
	Q_INVOKABLE static QColor getUserColor(const QString &nickName);

private:
	/**
	 * Highlights links in a list of words
	 */
	static QString processMsgFormatting(const QStringList &words, bool isFirst = true);
};

#endif // QMLUTILS_H
