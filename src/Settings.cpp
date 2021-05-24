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

#include "Settings.h"

Settings::Settings(QObject *parent)
	: QObject(parent)
	, m_settings(QStringLiteral(APPLICATION_NAME), QStringLiteral(APPLICATION_NAME))
{

}

QSettings &Settings::raw()
{
	return m_settings;
}

bool Settings::authOnline() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_AUTH_ONLINE), true).toBool();
}

void Settings::setAuthOnline(bool online)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_AUTH_ONLINE), online);
	emit authOnlineChanged();
}

QString Settings::authJid() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_AUTH_JID)).toString();
}

void Settings::setAuthJid(const QString &jid)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_AUTH_JID), jid);
	emit authJidChanged();
}

QString Settings::authJidResourcePrefix() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_AUTH_JID_RESOURCE_PREFIX), KAIDAN_JID_RESOURCE_DEFAULT_PREFIX).toString();
}

void Settings::setAuthJidResourcePrefix(const QString &prefix)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_AUTH_JID_RESOURCE_PREFIX), prefix);
	emit authJidResourcePrefixChanged();
}

QString Settings::authPassword() const
{
	return QByteArray::fromBase64(m_settings.value(QStringLiteral(KAIDAN_SETTINGS_AUTH_PASSWD)).toString().toUtf8());
}

void Settings::setAuthPassword(const QString &password)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_AUTH_PASSWD), QString::fromUtf8(password.toUtf8().toBase64()));
	emit authPasswordChanged();
}

QString Settings::authHost() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_AUTH_HOST)).toString();
}

void Settings::setAuthHost(const QString &host)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_AUTH_HOST), host);
	emit authHostChanged();
}

void Settings::resetAuthHost()
{
	m_settings.remove(QStringLiteral(KAIDAN_SETTINGS_AUTH_PORT));
}

quint16 Settings::authPort() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_AUTH_PORT), NON_CUSTOM_PORT).value<quint16>();
}

void Settings::setAuthPort(quint16 port)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_AUTH_PORT), port);
	emit authPortChanged();
}

void Settings::resetAuthPort()
{
	m_settings.remove(QStringLiteral(KAIDAN_SETTINGS_AUTH_PORT));
}

bool Settings::isDefaultAuthPort()
{
	return authPort() == NON_CUSTOM_PORT;
}

Kaidan::PasswordVisibility Settings::authPasswordVisibility() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_AUTH_PASSWD_VISIBILITY), Kaidan::PasswordVisible).value<Kaidan::PasswordVisibility>();
}

void Settings::setAuthPasswordVisibility(Kaidan::PasswordVisibility visibility)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_AUTH_PASSWD_VISIBILITY), visibility);
	emit authPasswordVisibilityChanged();
}

bool Settings::notificationsMuted(const QString &bareJid) const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_NOTIFICATIONS_MUTED) + bareJid, false).toBool();
}

void Settings::setNotificationsMuted(const QString &bareJid, bool muted)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_NOTIFICATIONS_MUTED) + bareJid, muted);
	emit notificationsMutedChanged(bareJid);
}

QStringList Settings::favoriteEmojis() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_FAVORITE_EMOJIS)).toStringList();
}

void Settings::setFavoriteEmojis(const QStringList &emoji)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_FAVORITE_EMOJIS), emoji);
	emit favoriteEmojisChanged();
}

QSize Settings::windowSize() const
{
	return m_settings.value(QStringLiteral(KAIDAN_SETTINGS_WINDOW_SIZE)).toSize();
}

void Settings::setWindowSize(const QSize size)
{
	m_settings.setValue(QStringLiteral(KAIDAN_SETTINGS_WINDOW_SIZE), size);
	emit windowSizeChanged();
}

void Settings::remove(const QStringList &keys)
{
	for (const QString &key : keys)
		m_settings.remove(key);
}
