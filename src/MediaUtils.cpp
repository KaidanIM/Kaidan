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

#include "MediaUtils.h"

#include <QFileInfo>
#include <QRegularExpression>
#include <QTime>
#include <QUrl>

static QList<QMimeType> mimeTypes(const QList<QMimeType> &mimeTypes, const QString &parent);

const QMimeDatabase MediaUtils::s_mimeDB;
const QList<QMimeType> MediaUtils::s_allMimeTypes(MediaUtils::s_mimeDB.allMimeTypes());
const QList<QMimeType> MediaUtils::s_imageTypes(::mimeTypes(MediaUtils::s_allMimeTypes, QStringLiteral("image")));
const QList<QMimeType> MediaUtils::s_audioTypes(::mimeTypes(MediaUtils::s_allMimeTypes, QStringLiteral("audio")));
const QList<QMimeType> MediaUtils::s_videoTypes(::mimeTypes(MediaUtils::s_allMimeTypes, QStringLiteral("video")));
const QList<QMimeType> MediaUtils::s_documentTypes {
	s_mimeDB.mimeTypeForName(QStringLiteral("application/vnd.oasis.opendocument.presentation")),
	s_mimeDB.mimeTypeForName(QStringLiteral("application/vnd.oasis.opendocument.spreadsheet")),
	s_mimeDB.mimeTypeForName(QStringLiteral("application/vnd.oasis.opendocument.text")),
	s_mimeDB.mimeTypeForName(QStringLiteral("application/vnd.openxmlformats-officedocument.presentationml.presentation")),
	s_mimeDB.mimeTypeForName(QStringLiteral("application/vnd.openxmlformats-officedocument.spreadsheetml.sheet")),
	s_mimeDB.mimeTypeForName(QStringLiteral("application/vnd.openxmlformats-officedocument.wordprocessingml.document")),
	s_mimeDB.mimeTypeForName(QStringLiteral("application/pdf")),
	s_mimeDB.mimeTypeForName(QStringLiteral("text/plain"))
};
const QList<QMimeType> MediaUtils::s_geoTypes {
	s_mimeDB.mimeTypeForName(QStringLiteral("application/geo+json"))
};
const QRegularExpression MediaUtils::s_geoLocationRegExp(QStringLiteral("geo:([-+]?[0-9]*\\.?[0-9]+),([-+]?[0-9]*\\.?[0-9]+)"));

static QList<QMimeType> mimeTypes(const QList<QMimeType> &mimeTypes, const QString &parent) {
	QList<QMimeType> mimes;

	for (const QMimeType &mimeType: mimeTypes) {
		if (mimeType.name().section(QLatin1Char('/'), 0, 0) == parent) {
			mimes << mimeType;
		}
	}

	return  mimes;
}

static QString iconName(const QList<QMimeType> &mimeTypes) {
	if (!mimeTypes.isEmpty()) {
		for (const QMimeType &type: mimeTypes) {
			const QString name = type.iconName();

			if (!name.isEmpty()) {
				return name;
			}
		}
	}

	return QStringLiteral("application-octet-stream");
}

static QString filter(const QList<QMimeType> &mimeTypes)
{
	QStringList filters;

	filters.reserve(mimeTypes.size() * 2);

	for (const QMimeType &mimeType : mimeTypes) {
		QString filter = mimeType.filterString();

		if (filter.isEmpty()) {
			filters << QStringLiteral("*");
			continue;
		}

		const int start = filter.lastIndexOf(QLatin1Char('('));
		const int end   = filter.lastIndexOf(QLatin1Char(')'));
		Q_ASSERT(start != -1);
		Q_ASSERT(end != -1);

		filters << filter.mid(start + 1, filter.size() - start - 1 - 1).split(QLatin1Char(' '));
	}

	if (filters.size() > 1) {
		filters.removeDuplicates();
		filters.removeAll(QStringLiteral("*"));
	}

	filters.reserve(filters.size());

	return filters.join(QLatin1Char(' '));
}

static QString prettyFormat(int durationMsecs, QTime *outTime = nullptr) {
	const QTime time(QTime(0, 0, 0).addMSecs(durationMsecs));
	QString format;

	if (time.hour() > 0) {
		format.append(QStringLiteral("H"));
	}

	format.append(format.isEmpty() ? QStringLiteral("m") : QStringLiteral(":mm"));
	format.append(format.isEmpty() ? QStringLiteral("s") : QStringLiteral(":ss"));

	if (outTime) {
		*outTime = time;
	}

	return format;
}

QString MediaUtils::prettyDuration(int msecs)
{
	QTime time;
	const QString format = prettyFormat(msecs, &time);
	return time.toString(format);
}

QString MediaUtils::prettyDuration(int msecs, int durationMsecs)
{
	const QString format = prettyFormat(durationMsecs);
	return QTime(0, 0, 0).addMSecs(msecs).toString(format);
}

bool MediaUtils::isHttp(const QString &content)
{
	const QUrl url(content);
	return url.isValid() && url.scheme().startsWith(QStringLiteral("http"));
}

bool MediaUtils::isGeoLocation(const QString &content)
{
	return s_geoLocationRegExp.match(content).hasMatch();
}

QGeoCoordinate MediaUtils::locationCoordinate(const QString &content)
{
	const QRegularExpressionMatch match = s_geoLocationRegExp.match(content);
	return match.hasMatch()
		       ? QGeoCoordinate(match.captured(1).toDouble(), match.captured(2).toDouble())
		       : QGeoCoordinate();
}

bool MediaUtils::localFileAvailable(const QString &filePath)
{
	if (filePath.isEmpty()) {
		return false;
	}

	const QUrl url(filePath);
	return url.isValid() && url.isLocalFile()
		       ? localFileAvailable(url)
		       : QFile::exists(filePath);
}

bool MediaUtils::localFileAvailable(const QUrl &url)
{
	if (url.isValid() && url.isLocalFile()) {
		return localFileAvailable(url.toLocalFile());
	}

	return false;
}

QUrl MediaUtils::fromLocalFile(const QString &filePath)
{
	return QUrl::fromLocalFile(filePath);
}

QMimeType MediaUtils::mimeType(const QString &filePath)
{
	if (filePath.isEmpty()) {
		return {};
	}

	const QUrl url(filePath);
	return url.isValid() && !url.scheme().isEmpty()
		       ? mimeType(url)
		       : s_mimeDB.mimeTypeForFile(filePath);
}

QMimeType MediaUtils::mimeType(const QUrl &url)
{
	if (!url.isValid()) {
		return {};
	}

	if (url.isLocalFile()) {
		return mimeType(url.toLocalFile());
	}

	if (url.scheme().compare(QStringLiteral("geo")) == 0) {
		return s_geoTypes.first();
	}

	return mimeType(url.fileName());
}

QString MediaUtils::iconName(const QString &filePath)
{
	if (filePath.isEmpty()) {
		return {};
	}

	const QUrl url(filePath);
	return url.isValid() && !url.scheme().isEmpty()
		       ? iconName(url)
		       : ::iconName({ mimeType(filePath) });
}

QString MediaUtils::iconName(const QUrl &url)
{
	if (!url.isValid()) {
		return {};
	}

	if (url.isLocalFile()) {
		return iconName(url.toLocalFile());
	}

	return iconName(url.fileName());
}

QString MediaUtils::mediaTypeName(Enums::MessageType mediaType)
{
	switch (mediaType) {
	case Enums::MessageType::MessageAudio:
		return tr("Audio");
	case Enums::MessageType::MessageDocument:
		return tr("Document");
	case Enums::MessageType::MessageGeoLocation:
		return tr("Location");
	case Enums::MessageType::MessageImage:
		return tr("Image");
	case Enums::MessageType::MessageVideo:
		return tr("Video");
	case Enums::MessageType::MessageFile:
	case Enums::MessageType::MessageText:
	case Enums::MessageType::MessageUnknown:
		return tr("File");
	}
	return {};
}

QString MediaUtils::newMediaLabel(Enums::MessageType hint)
{
	switch (hint) {
	case Enums::MessageType::MessageImage:
		return tr("Take picture");
	case Enums::MessageType::MessageVideo:
		return tr("Record video");
	case Enums::MessageType::MessageAudio:
		return tr("Record voice");
	case Enums::MessageType::MessageGeoLocation:
		return tr("Send location");
	case Enums::MessageType::MessageText:
	case Enums::MessageType::MessageFile:
	case Enums::MessageType::MessageDocument:
	case Enums::MessageType::MessageUnknown:
		break;
	}

	Q_UNREACHABLE();
	return { };
}

QString MediaUtils::newMediaIconName(Enums::MessageType hint)
{
	switch (hint) {
	case Enums::MessageType::MessageImage:
		return QStringLiteral("camera-photo-symbolic");
	case Enums::MessageType::MessageVideo:
		return QStringLiteral("camera-video-symbolic");
	case Enums::MessageType::MessageAudio:
		return QStringLiteral("microphone");
	case Enums::MessageType::MessageGeoLocation:
		return QStringLiteral("gps");
	case Enums::MessageType::MessageText:
	case Enums::MessageType::MessageFile:
	case Enums::MessageType::MessageDocument:
	case Enums::MessageType::MessageUnknown:
		break;
	}

	Q_UNREACHABLE();
	return { };
}

QString MediaUtils::label(Enums::MessageType hint)
{
	switch (hint) {
	case Enums::MessageType::MessageFile:
		return tr("Choose file");
	case Enums::MessageType::MessageImage:
		return tr("Choose image");
	case Enums::MessageType::MessageVideo:
		return tr("Choose video");
	case Enums::MessageType::MessageAudio:
		return tr("Choose audio file");
	case Enums::MessageType::MessageDocument:
		return tr("Choose document");
	case Enums::MessageType::MessageGeoLocation:
	case Enums::MessageType::MessageText:
	case Enums::MessageType::MessageUnknown:
		break;
	}

	Q_UNREACHABLE();
	return { };
}

QString MediaUtils::iconName(Enums::MessageType hint)
{
	return ::iconName(mimeTypes(hint));
}

QString MediaUtils::filterName(Enums::MessageType hint)
{
	switch (hint) {
	case Enums::MessageType::MessageText:
		break;
	case Enums::MessageType::MessageFile:
		return tr("All files");
	case Enums::MessageType::MessageImage:
		return tr("Images");
	case Enums::MessageType::MessageVideo:
		return tr("Videos");
	case Enums::MessageType::MessageAudio:
		return tr("Audio files");
	case Enums::MessageType::MessageDocument:
		return tr("Documents");
	case Enums::MessageType::MessageGeoLocation:
	case Enums::MessageType::MessageUnknown:
		break;
	}

	Q_UNREACHABLE();
	return { };
}

QString MediaUtils::filter(Enums::MessageType hint)
{
	return ::filter(mimeTypes(hint));
}

QString MediaUtils::namedFilter(Enums::MessageType hint)
{
	switch (hint) {
	case Enums::MessageType::MessageText:
		break;
	case Enums::MessageType::MessageFile:
	case Enums::MessageType::MessageImage:
	case Enums::MessageType::MessageVideo:
	case Enums::MessageType::MessageAudio:
	case Enums::MessageType::MessageDocument:
		return tr("%1 (%2)").arg(filterName(hint), filter(hint));
	case Enums::MessageType::MessageGeoLocation:
	case Enums::MessageType::MessageUnknown:
		break;
	}

	Q_UNREACHABLE();
	return { };
}

QList<QMimeType> MediaUtils::mimeTypes(Enums::MessageType hint)
{
	switch (hint) {
	case Enums::MessageType::MessageImage:
		return s_imageTypes;
	case Enums::MessageType::MessageVideo:
		return s_videoTypes;
	case Enums::MessageType::MessageAudio:
		return s_audioTypes;
	case Enums::MessageType::MessageDocument:
		return s_documentTypes;
	case Enums::MessageType::MessageGeoLocation:
		return s_geoTypes;
	case Enums::MessageType::MessageText:
	case Enums::MessageType::MessageUnknown:
		break;
	case Enums::MessageType::MessageFile:
		return { s_mimeDB.mimeTypeForName(QStringLiteral("application/octet-stream")) };
	}

	Q_UNREACHABLE();
	return { };
}

Enums::MessageType MediaUtils::messageType(const QString &filePath)
{
	if (filePath.isEmpty()) {
		return Enums::MessageType::MessageUnknown;
	}

	const QUrl url(filePath);
	return url.isValid() && !url.scheme().isEmpty()
		       ? messageType(url)
		       : messageType(s_mimeDB.mimeTypeForFile(filePath));
}

Enums::MessageType MediaUtils::messageType(const QUrl &url)
{
	if (!url.isValid()) {
		return Enums::MessageType::MessageUnknown;
	}

	if (url.isLocalFile()) {
		return messageType(url.toLocalFile());
	}

	QList<QMimeType> mimeTypes;

	if (url.scheme().compare(QStringLiteral("geo")) == 0) {
		mimeTypes = s_geoTypes;
	} else {
		const QFileInfo fileInfo(url.fileName());

		if (fileInfo.completeSuffix().isEmpty()) {
			return Enums::MessageType::MessageUnknown;
		}

		mimeTypes = s_mimeDB.mimeTypesForFileName(fileInfo.fileName());
	}

	for (const QMimeType &mimeType : std::as_const(mimeTypes)) {
		const Enums::MessageType messageType = MediaUtils::messageType(mimeType);

		if (messageType != Enums::MessageType::MessageUnknown) {
			return messageType;
		}
	}

	return Enums::MessageType::MessageUnknown;
}

Enums::MessageType MediaUtils::messageType(const QMimeType &mimeType)
{
	if (!mimeType.isValid()) {
		return Enums::MessageType::MessageUnknown;
	}

	if (mimeTypes(Enums::MessageType::MessageImage).contains(mimeType)) {
		return Enums::MessageType::MessageImage;
	} else if (mimeTypes(Enums::MessageType::MessageAudio).contains(mimeType)) {
		return Enums::MessageType::MessageAudio;
	} else if (mimeTypes(Enums::MessageType::MessageVideo).contains(mimeType)) {
		return Enums::MessageType::MessageVideo;
	} else if (mimeTypes(Enums::MessageType::MessageGeoLocation).contains(mimeType)) {
		return Enums::MessageType::MessageGeoLocation;
	} else if (mimeTypes(Enums::MessageType::MessageDocument).contains(mimeType)) {
		return Enums::MessageType::MessageDocument;
	}

	return Enums::MessageType::MessageFile;
}
