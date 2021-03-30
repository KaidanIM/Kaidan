/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
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

#include "QmlUtils.h"
// Qt
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QGuiApplication>
#include <QImage>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QStringBuilder>
// QXmpp
#include "qxmpp-exts/QXmppColorGenerator.h"

static QmlUtils *s_instance;

QmlUtils *QmlUtils::instance()
{
	if (!s_instance)
		return new QmlUtils(QGuiApplication::instance());

	return s_instance;
}

QmlUtils::QmlUtils(QObject *parent)
	: QObject(parent)
{
	Q_ASSERT(!s_instance);
	s_instance = this;
}

QmlUtils::~QmlUtils()
{
	s_instance = nullptr;
}

QString QmlUtils::connectionErrorMessage(ClientWorker::ConnectionError error)
{
	switch (error) {
	case ClientWorker::NoError:
		break;
	case ClientWorker::AuthenticationFailed:
		return tr("Invalid username or password.");
	case ClientWorker::NotConnected:
		return tr("Cannot connect to the server. Please check your internet connection.");
	case ClientWorker::TlsFailed:
		return tr("Error while trying to connect securely.");
	case ClientWorker::TlsNotAvailable:
		return tr("The server doesn't support secure connections.");
	case ClientWorker::DnsError:
		return tr("Could not connect to the server. Please check your internet connection or your server name.");
	case ClientWorker::ConnectionRefused:
		return tr("The server is offline or blocked by a firewall.");
	case ClientWorker::NoSupportedAuth:
		return tr("Authentification protocol not supported by the server.");
	case ClientWorker::KeepAliveError:
		return tr("The connection could not be refreshed.");
	case ClientWorker::NoNetworkPermission:
		return tr("The internet access is not permitted. Please check your system's internet access configuration.");
	case ClientWorker::RegistrationUnsupported:
		return tr("This server does not support registration.");
	}
	Q_UNREACHABLE();
}

QString QmlUtils::getResourcePath(const QString &name)
{
	// We generally prefer to first search for files in application resources
	if (QFile::exists(":/" + name))
		return QString("qrc:/" + name);

	// list of file paths where to search for the resource file
	QStringList pathList;
	// add relative path from binary (only works if installed)
	pathList << QCoreApplication::applicationDirPath() + QString("/../share/") + QString(APPLICATION_NAME);
	// get the standard app data locations for current platform
	pathList << QStandardPaths::standardLocations(QStandardPaths::AppDataLocation);
#ifdef UBUNTU_TOUCH
	pathList << QString("./share/") + QString(APPLICATION_NAME);
#endif
#ifndef NDEBUG
#ifdef DEBUG_SOURCE_PATH
	// add source directory (only for debug builds)
	pathList << QString(DEBUG_SOURCE_PATH) + QString("/data");
#endif
#endif

	// search for file in directories
	for (int i = 0; i < pathList.size(); i++) {
		// open directory
		QDir directory(pathList.at(i));
		// look up the file
		if (directory.exists(name)) {
			// found the file, return the path
			return QUrl::fromLocalFile(directory.absoluteFilePath(name)).toString();
		}
	}

	// no file found
	qWarning() << "[main] Could NOT find media file:" << name;
	return QString();
}

QUrl QmlUtils::issueTrackingUrl()
{
	return {QStringLiteral(ISSUE_TRACKING_URL)};
}

bool QmlUtils::isImageFile(const QUrl &fileUrl)
{
	QMimeType type = QMimeDatabase().mimeTypeForUrl(fileUrl);
	return type.inherits("image/jpeg") || type.inherits("image/png");
}

void QmlUtils::copyToClipboard(const QString &text)
{
	QGuiApplication::clipboard()->setText(text);
}

QString QmlUtils::fileNameFromUrl(const QUrl &url)
{
	return QUrl(url).fileName();
}

QString QmlUtils::fileSizeFromUrl(const QUrl &url)
{
	return QLocale::system().formattedDataSize(
	            QFileInfo(QUrl(url).toLocalFile()).size());
}

QString QmlUtils::formatMessage(const QString &message)
{
	// escape all special XML chars (like '<' and '>')
	// and spilt into words for processing
	return processMsgFormatting(message.toHtmlEscaped().split(" "));
}

QColor QmlUtils::getUserColor(const QString &nickName)
{
	QXmppColorGenerator::RGBColor color = QXmppColorGenerator::generateColor(nickName);
	return {color.red, color.green, color.blue};
}

QUrl QmlUtils::pasteImage()
{
	const auto image = QGuiApplication::clipboard()->image();
	if (image.isNull())
		return {};

	// create absolute file path
	const auto path = downloadPath(u"image-" % timestampForFileName() % u".jpg");

	// encode JPEG image
	if (!image.save(path, "JPG", JPEG_EXPORT_QUALITY))
		return {};
	return QUrl::fromLocalFile(path);
}

QString QmlUtils::downloadPath(const QString &filename)
{
	// Kaidan download directory
	const QDir directory(
			QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) %
				QDir::separator() %
				APPLICATION_DISPLAY_NAME);
	// create directory if it doesn't exist
	if (!directory.mkpath("."))
		return {};

	// check whether a file with this name already exists
	QFileInfo info(directory, filename);
	if (!info.exists())
		return info.absoluteFilePath();

	// find a new filename that doesn't exist
	const auto baseName = info.baseName();
	const auto nameSuffix = info.suffix();
	for (uint i = 1; info.exists(); i++) {
		// try to insert '-<i>' before the file extension
		info = QFileInfo(directory, baseName % u'-' % QString::number(i) % nameSuffix);
	}
	return info.absoluteFilePath();
}

QString QmlUtils::timestampForFileName()
{
	return timestampForFileName(QDateTime::currentDateTime());
}

QString QmlUtils::timestampForFileName(const QDateTime &dateTime)
{
	return dateTime.toString(u"yyyyMMdd_hhmmss");
}

QString QmlUtils::chatStateDescription(const QString &displayName, const QXmppMessage::State state)
{
	switch (state) {
	case QXmppMessage::State::Active:
		return tr("%1 is online").arg(displayName);
	case QXmppMessage::State::Composing:
		return tr("%1 is typing…").arg(displayName);
	case QXmppMessage::State::Paused:
		return tr("%1 paused typing").arg(displayName);
	// Not helpful for the user, so don't display
	case QXmppMessage::State::Gone:
	case QXmppMessage::State::Inactive:
	case QXmppMessage::State::None:
		break;
	};

	return {};
}

QString QmlUtils::processMsgFormatting(const QStringList &list, bool isFirst)
{
	if (list.isEmpty())
		return QString();

	// link highlighting
	if (list.first().startsWith("https://") || list.first().startsWith("http://"))
		return (isFirst ? QString() : " ") + QString("<a href='%1'>%1</a>").arg(list.first())
		       + processMsgFormatting(list.mid(1), false);

	// preserve newlines
	if (list.first().contains("\n"))
		return (isFirst ? QString() : " ") + QString(list.first()).replace("\n", "<br>")
		       + processMsgFormatting(list.mid(1), false);

	return (isFirst ? QString() : " ") + list.first() + processMsgFormatting(list.mid(1), false);
}
