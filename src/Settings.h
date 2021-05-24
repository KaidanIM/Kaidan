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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QSettings>
#include <QSize>

#include "Globals.h"
#include "Kaidan.h"

constexpr quint16 NON_CUSTOM_PORT = 0;

class Settings : public QObject
{
    Q_OBJECT

public:
    explicit Settings(QObject *parent = nullptr);

    ///
    /// Avoid using this in favour of adding methods here,
    /// but it is useful if you need to manually manage config groups
    ///
    QSettings &raw();

    bool authOnline() const;
    void setAuthOnline(bool online);

    QString authJid() const;
    void setAuthJid(const QString &jid);

    QString authJidResourcePrefix() const;
    void setAuthJidResourcePrefix(const QString &prefix);

    QString authPassword() const;
    void setAuthPassword(const QString &password);

    QString authHost() const;
    void setAuthHost(const QString &host);
    void resetAuthHost();

    quint16 authPort() const;
    void setAuthPort(quint16 port);
    void resetAuthPort();
    bool isDefaultAuthPort();

    Kaidan::PasswordVisibility authPasswordVisibility() const;
    void setAuthPasswordVisibility(Kaidan::PasswordVisibility visibility);

    bool notificationsMuted(const QString &bareJid) const;
    void setNotificationsMuted(const QString &bareJid, bool muted);

    QStringList favoriteEmojis() const;
    void setFavoriteEmojis(const QStringList &emoji);

    QSize windowSize() const;
    void setWindowSize(const QSize size);

    void remove(const QStringList &keys);

signals:
    void authOnlineChanged();
    void authJidChanged();
    void authJidResourcePrefixChanged();
    void authPasswordChanged();
    void authHostChanged();
    void authPortChanged();
    void authPasswordVisibilityChanged();
    void notificationsMutedChanged(const QString &bareJid);
    void favoriteEmojisChanged();
    void windowSizeChanged();

private:
    QSettings m_settings;
};

#endif // SETTINGS_H
