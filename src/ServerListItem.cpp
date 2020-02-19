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

#include "ServerListItem.h"
// Qt
#include <QLocale>
#include <QJsonObject>
#include <QUrl>

#define REGIONAL_INDICATOR_SYMBOL_BASE 0x1F1A5

class ServerListItemPrivate : public QSharedData
{
public:
	ServerListItemPrivate();

	bool isCustomServer;
	QString jid;
	bool supportsInBandRegistration;
	QUrl registrationWebPage;
	QString language;
	QString country;
	QUrl website;
	int onlineSince;
	int httpUploadSize;
	int messageStorageDuration;
};

ServerListItemPrivate::ServerListItemPrivate()
    : isCustomServer(false), supportsInBandRegistration(false), onlineSince(-1), httpUploadSize(-1), messageStorageDuration(-1)
{
}

ServerListItem ServerListItem::fromJson(const QJsonObject &object)
{
	ServerListItem item;
	item.setIsCustomServer(false);
	item.setJid(object.value(QLatin1String("jid")).toString());
	item.setSupportsInBandRegistration(object.value(QLatin1String("supportsInBandRegistration")).toBool());
	item.setRegistrationWebPage(QUrl(object.value(QLatin1String("registrationWebPage")).toString()));
	item.setLanguage(object.value(QLatin1String("language")).toString().toUpper());
	item.setCountry(object.value(QLatin1String("country")).toString().toUpper());
	item.setWebsite(QUrl(object.value(QLatin1String("website")).toString()));
	item.setOnlineSince(object.value(QLatin1String("onlineSince")).toInt(-1));
	item.setHttpUploadSize(object.value(QLatin1String("httpUploadSize")).toInt(-1));
	item.setMessageStorageDuration(object.value(QLatin1String("messageStorageDuration")).toInt(-1));
	return item;
}

ServerListItem::ServerListItem(bool isCustomServer)
	: d(new ServerListItemPrivate)
{
	d->isCustomServer = isCustomServer;
}

ServerListItem::ServerListItem(const ServerListItem& other) = default;

ServerListItem::~ServerListItem() = default;

ServerListItem & ServerListItem::operator=(const ServerListItem& other) = default;

bool ServerListItem::isCustomServer() const
{
	return d->isCustomServer;
}

void ServerListItem::setIsCustomServer(bool isCustomServer)
{
	d->isCustomServer = isCustomServer;
}

QString ServerListItem::jid() const
{
	return d->jid;
}

void ServerListItem::setJid(const QString &jid)
{
	d->jid = jid;
}

bool ServerListItem::supportsInBandRegistration() const
{
	return d->supportsInBandRegistration;
}

void ServerListItem::setSupportsInBandRegistration(bool supportsInBandRegistration)
{
	d->supportsInBandRegistration = supportsInBandRegistration;
}

QUrl ServerListItem::registrationWebPage() const
{
	return d->registrationWebPage;
}

void ServerListItem::setRegistrationWebPage(const QUrl &registrationWebPage)
{
	d->registrationWebPage = registrationWebPage;
}

QString ServerListItem::language() const
{
	return d->language;
}

void ServerListItem::setLanguage(const QString &language)
{
	d->language = language;
}

QString ServerListItem::country() const
{
	return d->country;
}

void ServerListItem::setCountry(const QString &country)
{
	d->country = country;
}

QString ServerListItem::flag() const
{
	// If this object is the custom server, no flag should be shown.
	if (d->isCustomServer)
		return {};

	// If the country is not specified, return a flag for an unknown country.
	if (d->country.isEmpty())
		return QStringLiteral("🏳️‍🌈");

	QString flag;

	// Iterate over the characters of the country string.
	// Example: For the country string "DE" the loop iterates over the characters "D" and "E".
	// An emoji flag sequence (i.e. the flag of the corresponding country / region) is represented by two regional indicator symbols.
	// Example: 🇩 (U+1F1E9 = 0x1F1E9 = 127465) and 🇪 (U+1F1EA = 127466) concatenated result in 🇩🇪.
	// Each regional indicator symbol is created by a string which has the following Unicode code point:
	// REGIONAL_INDICATOR_SYMBOL_BASE + unicode code point of the character of the country string.
	// Example: 127397 (REGIONAL_INDICATOR_SYMBOL_BASE) + 68 (unicode code point of "D") = 127465 for 🇩
	//
	// QString does not provide to create a string by its corresponding Unicode code point.
	// Therefore, QChar must be used to create a character by its Unicode code point.
	// Unfortunately, that can be done in one step because QChar does not support creating Unicode characters greater than 16 bits.
	// For this reason, each character of the country string is split into two parts.
	// Each part consists of 16 bits of the original character.
	// The first and the second part are then merged into one string.
	//
	// Finally, the string consisting of the first regional indicator symbol and the string consisting the second one are concatenated.
	// The resulting string represents the emoji flag sequence.
	for (const auto &character : d->country) {
		uint32_t regionalIncidatorSymbolCodePoint = REGIONAL_INDICATOR_SYMBOL_BASE + character.unicode();
		QString regionalIncidatorSymbol;

		QChar regionalIncidatorSymbolParts[2];
		regionalIncidatorSymbolParts[0] = QChar::highSurrogate(regionalIncidatorSymbolCodePoint);
		regionalIncidatorSymbolParts[1] = QChar::lowSurrogate(regionalIncidatorSymbolCodePoint);

		regionalIncidatorSymbol = QString(regionalIncidatorSymbolParts, 2);

		flag.append(regionalIncidatorSymbol);
	}

	return flag;
}

QUrl ServerListItem::website() const
{
	return d->website;
}

void ServerListItem::setWebsite(const QUrl &website)
{
	d->website = website;
}

int ServerListItem::onlineSince() const
{
	return d->onlineSince;
}

void ServerListItem::setOnlineSince(int onlineSince)
{
	d->onlineSince = onlineSince;
}

int ServerListItem::httpUploadSize() const
{
	return d->httpUploadSize;
}

void ServerListItem::setHttpUploadSize(int httpUploadSize)
{
	d->httpUploadSize = httpUploadSize;
}

int ServerListItem::messageStorageDuration() const
{
	return d->messageStorageDuration;
}

void ServerListItem::setMessageStorageDuration(int messageStorageDuration)
{
	d->messageStorageDuration = messageStorageDuration;
}

bool ServerListItem::operator<(const ServerListItem& other) const
{
	return d->jid < other.jid();
}

bool ServerListItem::operator>(const ServerListItem& other) const
{
	return d->jid > other.jid();
}

bool ServerListItem::operator<=(const ServerListItem& other) const
{
	return d->jid <= other.jid();
}

bool ServerListItem::operator>=(const ServerListItem& other) const
{
	return d->jid >= other.jid();
}

bool ServerListItem::operator==(const ServerListItem& other) const
{
	return d == other.d;
}
