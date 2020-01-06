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

#pragma once

#include <QMimeDatabase>
#include <QObject>
#include <QGeoCoordinate>

#include "Enums.h"

class MediaUtils : public QObject
{
	Q_OBJECT

public:
	using QObject::QObject;

	Q_INVOKABLE static QString prettyDuration(int msecs);
	Q_INVOKABLE static QString prettyDuration(int msecs, int durationMsecs);
	Q_INVOKABLE static bool isHttp(const QString &content);
	Q_INVOKABLE static bool isGeoLocation(const QString &content);
	Q_INVOKABLE static QGeoCoordinate locationCoordinate(const QString &content);
	Q_INVOKABLE static bool localFileAvailable(const QString &filePath);
	Q_INVOKABLE static bool localFileAvailable(const QUrl &url);
	Q_INVOKABLE static QUrl fromLocalFile(const QString &filePath);
	Q_INVOKABLE static QMimeType mimeType(const QString &filePath);
	Q_INVOKABLE static QMimeType mimeType(const QUrl &url);
	Q_INVOKABLE static QString iconName(const QString &filePath);
	Q_INVOKABLE static QString iconName(const QUrl &url);

	Q_INVOKABLE static QString newMediaLabel(Enums::MessageType hint);
	Q_INVOKABLE static QString newMediaIconName(Enums::MessageType hint);
	Q_INVOKABLE static QString label(Enums::MessageType hint);
	Q_INVOKABLE static QString iconName(Enums::MessageType hint);
	Q_INVOKABLE static QString filterName(Enums::MessageType hint);
	Q_INVOKABLE static QString filter(Enums::MessageType hint);
	Q_INVOKABLE static QString namedFilter(Enums::MessageType hint);
	Q_INVOKABLE static QList<QMimeType> mimeTypes(Enums::MessageType hint);
	Q_INVOKABLE static Enums::MessageType messageType(const QString &filePath);
	Q_INVOKABLE static Enums::MessageType messageType(const QUrl &url);
	Q_INVOKABLE static Enums::MessageType messageType(const QMimeType &mimeType);

	// Qml sucks... :)
	Q_INVOKABLE inline static QString mimeTypeName(const QString &filePath)
	{ return mimeType(filePath).name(); }

	Q_INVOKABLE inline static QString mimeTypeName(const QUrl &url)
	{ return mimeType(url).name(); }

private:
	static const QMimeDatabase s_mimeDB;
	static const QList<QMimeType> s_allMimeTypes;
	static const QList<QMimeType> s_imageTypes;
	static const QList<QMimeType> s_audioTypes;
	static const QList<QMimeType> s_videoTypes;
	static const QList<QMimeType> s_documentTypes;
	static const QList<QMimeType> s_geoTypes;
	static const QRegularExpression s_geoLocationRegExp;
};
