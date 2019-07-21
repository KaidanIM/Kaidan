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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>
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
	QmlUtils(QObject *parent = nullptr);

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

public slots:
	/**
	 * Returns a URL to a given resource file name
	 *
	 * This will check various paths which could contain the searched file.
	 * If the file was found, it'll return a `file://` or a `qrc:/` url to
	 * the file.
	 */
	QString getResourcePath(const QString &resourceName) const;

	/**
	 * Returns a string of this build's Kaidan version
	 */
	QString getVersionString() const
	{
		return VERSION_STRING;
	}

	/**
	 * Returns a string without new lines, unneeded spaces, etc.
	 *
	 * See QString::simplified for more information.
	 */
	QString removeNewLinesFromString(const QString &input) const
	{
		return input.simplified();
	}

	/**
	 * Checks whether a file is an image and could be displayed as such.
	 * @param fileUrl URL to the possible image file
	 */
	bool isImageFile(const QUrl &fileUrl) const;

	/**
	 * Copy text to the clipboard
	 */
	void copyToClipboard(const QString &text) const;

	/**
	 * Returns the file name from a URL
	 */
	QString fileNameFromUrl(const QUrl &url) const;

	/**
	 * Returns the file size from a URL
	 */
	QString fileSizeFromUrl(const QUrl &url) const;

	/**
	 * Styles/formats a message for displaying
	 *
	 * This currently only adds some link highlighting
	 */
	QString formatMessage(const QString &message) const;

	/**
	 * Returns a consistent user color generated from the nickname.
	 */
	QColor getUserColor(const QString &nickName) const;

private:
	/**
	 * Highlights links in a list of words
	 */
	QString processMsgFormatting(const QStringList &words, bool isFirst = true) const;
};

#endif // UTILS_H
