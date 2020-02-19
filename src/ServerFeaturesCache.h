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

#ifndef SERVERFEATURESCACHE_H
#define SERVERFEATURESCACHE_H

#include <QObject>
#include <QMutex>

/**
 * @brief The ServerFeaturesCache class temporarily stores the features of a server. This
 * can be used to for example enable or disable certain features in the UI.
 *
 * All methods in the class are thread-safe.
 */
class ServerFeaturesCache : public QObject
{
	Q_OBJECT
	Q_PROPERTY(bool inBandRegistrationSupported READ inBandRegistrationSupported NOTIFY inBandRegistrationSupportedChanged)
	Q_PROPERTY(bool httpUploadSupported READ httpUploadSupported NOTIFY httpUploadSupportedChanged)

public:
	explicit ServerFeaturesCache(QObject *parent = nullptr);

	/**
	 * Returns whether In-Band Registration features after login on the server are supported by it.
	 */
	bool inBandRegistrationSupported();

	/**
	 * Sets whether In-Band Registration is supported.
	 */
	void setInBandRegistrationSupported(bool supported);

	/**
	 * Returns whether HTTP File Upload is available and can be currently be used.
	 */
	bool httpUploadSupported();
	void setHttpUploadSupported(bool supported);

signals:
	/**
	 * Emitted when In-Band Registration support changed.
	 */
	bool inBandRegistrationSupportedChanged();

	void httpUploadSupportedChanged();

private:
	QMutex m_mutex;
	bool m_inBandRegistrationSupported = false;
	bool m_httpUploadSupported = false;
};

#endif // SERVERFEATURESCACHE_H
