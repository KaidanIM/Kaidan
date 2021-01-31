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

#pragma once

// Qt
#include <QMutex>
#include <QObject>

class QSettings;

constexpr quint16 NON_CUSTOM_PORT = 0;

/**
 * This class manages account-related settings.
 */
class AccountManager : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString jid READ jid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(QString password READ password WRITE setPassword NOTIFY passwordChanged)
	Q_PROPERTY(QString host READ host WRITE setHost NOTIFY hostChanged)
	Q_PROPERTY(quint16 port READ port WRITE setPort NOTIFY portChanged)
	Q_PROPERTY(quint16 nonCustomPort READ nonCustomPort CONSTANT)

public:
	static AccountManager *instance();

	AccountManager(QSettings *settings, QObject *parent = nullptr);

	/**
	 * Returns the bare JID of the account.
	 *
	 * This method is thread-safe.
	 */
	QString jid();

	/**
	 * Sets the bare JID of the account.
	 *
	 * This method is thread-safe.
	 *
	 * @param jid bare JID of the account
	 */
	void setJid(const QString &jid);

	/**
	 * Returns the resource part of the account's full JID.
	 */
	QString jidResource() const;

	/**
	 * Sets the prefix of the account's full JID's resource part.
	 *
	 * The resource prefix is used to create the complete resource. The resource is created by
	 * appending a dot and random alphanumeric characters.
	 *
	 * Example:
	 *  Full JID: alice@example.org/Kaidan.DzF9
	 *  Resource prefix: Kaidan
	 *
	 * @param jidResourcePrefix prefix of the account's full JID's resource part
	 */
	void setJidResourcePrefix(const QString &jidResourcePrefix);

	/**
	 * Returns the password of the account.
	 *
	 * This method is thread-safe.
	 */
	QString password();

	/**
	 * Sets the password of the account.
	 *
	 * This method is thread-safe.
	 *
	 * @param password password of the account
	 */
	void setPassword(const QString &password);

	/**
	 * Returns the custom host.
	 *
	 * This method is thread-safe.
	 *
	 * @return the custom host or an empty string if no custom host is set
	 */
	QString host();

	/**
	 * Sets a custom host for connecting.
	 *
	 * This method is thread-safe.
	 *
	 * @param host host to connect to
	 */
	void setHost(const QString &host);

	/**
	 * Returns the custom port.
	 *
	 * This method is thread-safe.
	 *
	 * @return the custom port or nonCustomPort if no custom port is set
	 */
	quint16 port();

	/**
	 * Sets a custom port for connecting.
	 *
	 * This method is thread-safe.
	 *
	 * @param port port to connect to
	 */
	void setPort(const quint16 port);

	/**
	 * Returns the port which indicates that no custom port is set.
	 */
	quint16 nonCustomPort() const;

	/**
	 * Resets the custom connection settings.
	 */
	Q_INVOKABLE void resetCustomConnectionSettings();

	/**
	 * Provides a way to cache whether the current credentials are new to this client.
	 *
	 * The credentials are new to the client if they were not already in use. That is the case
	 * after entering the credentials the first time to log in or on the first login after
	 * registration.
	 *
	 * @return true if the credentials are new, otherwise false
	 */
	bool hasNewCredentials() const;

	/**
	 * Sets whether the current credentials are new to this client.
	 *
	 * @param hasNewCredentials true if the credentials are new, otherwise false
	 */
	void setHasNewCredentials(bool hasNewCredentials);

	/**
	 * Returns whether there are enough credentials available to log in to the server.
	 */
	bool hasEnoughCredentialsForLogin();

	/**
	 * Loads all credentials used to connect to the server.
	 *
	 * If credentials are missing for a login, the signal credentialsNeeded() is emitted.
	 *
	 * @return true if the credentials could be loaded, otherwise false
	 */
	Q_INVOKABLE bool loadCredentials();

	/**
	 * Stores the currently set JID in the settings file.
	 */
	void storeJid();

	/**
	 * Stores the currently set password in the settings file.
	 */
	void storePassword();

	/**
	 * Stores the currently set custom host and port in the settings file.
	 *
	 * If a custom connection setting is not set, it is removed from the settings file.
	 */
	void storeCustomConnectionSettings();

	/**
	 * Stores credentials (jid, password, etc.) in the settings file.
	 */
	void storeCredentials();

	/**
	 * Deletes all credentials.
	 *
	 * Credentials stored in the settings file are also removed from it.
	 */
	void deleteCredentials();

	/**
	 * Deletes all account related settings.
	 *
	 * Settings stored in the settings file are also removed from it.
	 */
 	void deleteSettings();

signals:
	/**
	 * Emitted when the JID changed.
	 */
	void jidChanged();

	/**
	 * Emitted when the password changed.
	 */
	void passwordChanged();

	/**
	 * Emitted when the custom host changed.
	 */
	void hostChanged();

	/**
	 * Emitted when the custom port changed.
	 */
	void portChanged();

	/**
	 * Emitted when there are no (correct) credentials to log in.
	 */
	void credentialsNeeded();

private:
	/**
	 * Deletes settings in the settings / configuration file.
	 *
	 * @param keys keys of the settings being deleted
	 */
	void deleteSettingsInSettingsFile(const QStringList &keys) const;

	/**
	 * Generates the JID's resource part with the set JID resource prefix and a suffix
	 * consisting of a dot followed by random alphanumeric characters.
	 *
	 * @param numberOfRandomSuffixCharacters number of random alphanumeric characters the
	 * suffix should consist of after the dot
	 */
	QString generateJidResourceWithRandomSuffix(unsigned int numberOfRandomSuffixCharacters = 4) const;

	QMutex m_mutex;
	QSettings *m_settings;

	QString m_jid;
	QString m_jidResourcePrefix;
	QString m_jidResource;
	QString m_password;
	QString m_host;
	quint16 m_port = NON_CUSTOM_PORT;

	bool m_hasNewCredentials;

	static AccountManager *s_instance;
};
