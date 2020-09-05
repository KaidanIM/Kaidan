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

// Kaidan
class ClientWorker;
#include "Globals.h"
class Kaidan;
class RegistrationDataFormModel;
// Qt
#include <QObject>
#include <QVector>
class QSettings;
// QXmpp
#include <QXmppBitsOfBinaryContentId.h>
class QXmppClient;
class QXmppDataForm;
class QXmppDiscoveryIq;
class QXmppRegisterIq;
#include <QXmppStanza.h>
class QXmppRegistrationManager;

class RegistrationManager : public QObject
{
	Q_OBJECT

public:
	enum RegistrationError {
		UnknownError,
		InBandRegistrationNotSupported,
		UsernameConflict,
		PasswordTooWeak,
		CaptchaVerificationFailed,
		RequiredInformationMissing,
		TemporarilyBlocked
	};
	Q_ENUM(RegistrationError)

	RegistrationManager(ClientWorker *clientWorker, QXmppClient *client, QSettings *settings);

	/**
	 * Sets whether a registration is requested for the next time when the client connects to the server.
	 *
	 * @param registerOnConnect true for requesting a registration on connecting, false otherwise
	 */
	void setRegisterOnConnectEnabled(bool registerOnConnect);

public slots:
	/**
	 * Sends the form containing information to register an account.
	 */
	void sendRegistrationForm();

	/**
	 * Deletes the account from the server.
	 */
	void deleteAccount();

	/**
	 * Changes the user's password.
	 *
	 * @param newPassword new password to set
	 */
	void changePassword(const QString &newPassword);

private:
	/**
	 * Called when the In-Band Registration support changed.
	 *
	 * The server feature state for In-Band Registration is only changed when the server disables it, not on disconnect.
	 * That way, the last known state can be cached while being offline and operations like deleting an account from the server can be offered to the user even if Kaidan is not connected to the user's server.
	 */
	void handleInBandRegistrationSupportedChanged();

	void handlePasswordChangeSucceeded(const QString &newPassword);
	void handlePasswordChangeFailed(const QXmppStanza::Error &error);

	/**
	 * Handles an incoming form used to register an account.
	 *
	 * @param iq IQ stanza to be handled
	 */
	void handleRegistrationFormReceived(const QXmppRegisterIq &iq);

	/**
	 * Handles a succeeded registration.
	 */
	void handleRegistrationSucceeded();

	/**
	 * Handles a failed registration.
	 *
	 * @param error error describing the reason for the failure
	 */
	void handleRegistrationFailed(const QXmppStanza::Error &error);

	/**
	 * Extracts a form from an IQ stanza for registration.
	 *
	 * @param iq IQ stanza containing the form
	 * @param isFakeForm true if the form is used to TODO: what?
	 *
	 * @return data form with extracted key-value pairs
	 */
	QXmppDataForm extractFormFromRegisterIq(const QXmppRegisterIq &iq, bool &isFakeForm);

	/**
	 * Copies values set by the user to a new form.
	 *
	 * @param oldForm form with old values
	 * @param newForm form with new values
	 */
	void copyUserDefinedValuesToNewForm(const QXmppDataForm &oldForm, QXmppDataForm &newForm);

	/**
	 * Cleans up the last form used for registration.
	 */
	void cleanUpLastForm();

	ClientWorker *m_clientWorker;
	QXmppClient *m_client;
	QSettings *m_settings;
	QXmppRegistrationManager *m_manager;
	RegistrationDataFormModel *m_dataFormModel;
	QVector<QXmppBitsOfBinaryContentId> m_contentIdsToRemove;
};
