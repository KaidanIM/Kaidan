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

#include "QmlUtils.h"
// Qt
#include <QClipboard>
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QGuiApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QUrl>
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

QString QmlUtils::presenceTypeToIcon(Enums::AvailabilityTypes type)
{
	switch (type) {
	case AvailabilityTypes::PresOnline:
		return "im-user-online";
	case AvailabilityTypes::PresChat:
		return "im-user-online";
	case AvailabilityTypes::PresAway:
		return "im-user-away";
	case AvailabilityTypes::PresDND:
		return "im-kick-user";
	case AvailabilityTypes::PresXA:
		return "im-user-away";
	case AvailabilityTypes::PresUnavailable:
		return "im-user-offline";
	case AvailabilityTypes::PresError:
		return "im-ban-kick-user";
	case AvailabilityTypes::PresInvisible:
		return "im-invisible-user";
	}
	Q_UNREACHABLE();
	return { };
}

QString QmlUtils::presenceTypeToText(AvailabilityTypes type)
{
	switch (type) {
	case AvailabilityTypes::PresOnline:
		return tr("Available");
	case AvailabilityTypes::PresChat:
		return tr("Free for chat");
	case AvailabilityTypes::PresAway:
		return tr("Away");
	case AvailabilityTypes::PresDND:
		return tr("Do not disturb");
	case AvailabilityTypes::PresXA:
		return tr("Away for longer");
	case AvailabilityTypes::PresUnavailable:
		return tr("Offline");
	case AvailabilityTypes::PresError:
		return tr("Error");
	case AvailabilityTypes::PresInvisible:
		return tr("Invisible");
	}
	Q_UNREACHABLE();
	return { };
}

QColor QmlUtils::presenceTypeToColor(AvailabilityTypes type)
{
	switch (type) {
	case AvailabilityTypes::PresOnline:
		return {"green"};
	case AvailabilityTypes::PresChat:
		return {"darkgreen"};
	case AvailabilityTypes::PresAway:
		return {"orange"};
	case AvailabilityTypes::PresDND:
		return QColor::fromRgb(218, 68, 83);
	case AvailabilityTypes::PresXA:
		return {"orange"};
	case AvailabilityTypes::PresError:
		return {"red"};
	case AvailabilityTypes::PresUnavailable:
		return {"silver"};
	case AvailabilityTypes::PresInvisible:
		return {"grey"};
	}
	Q_UNREACHABLE();
	return { };
}

QString QmlUtils::connectionErrorMessage(ClientWorker::ConnectionError error)
{
	switch (error) {
	case ClientWorker::AuthenticationFailed:
		return tr("Invalid username or password.");
	case ClientWorker::NotConnected:
		return tr("Cannot connect to the server. Please check your internet connection.");
	case ClientWorker::TlsFailed:
		return tr("Error while trying to connect securely.");
	case ClientWorker::TlsNotAvailable:
		return tr("The server doesn't support secure connections.");
	case ClientWorker::DnsError:
		return tr("Could not resolve the server's address. Please check your server name.");
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
