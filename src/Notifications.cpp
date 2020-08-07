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

#include "Notifications.h"

// KNotifications
#ifdef HAVE_KNOTIFICATIONS
#include <KNotification>
#endif

// Kaidan
#include "Kaidan.h"

#ifdef HAVE_KNOTIFICATIONS
static bool IS_USING_GNOME = qEnvironmentVariable("XDG_CURRENT_DESKTOP").contains("GNOME", Qt::CaseInsensitive);

void Notifications::sendMessageNotification(const QString &senderJid, const QString &senderName, const QString &message)
{
	KNotification *notification = new KNotification("new-message");
	notification->setText(message);
	notification->setTitle(senderName);
#ifdef Q_OS_UNIX
	if (IS_USING_GNOME)
		notification->setFlags(KNotification::Persistent);
#endif
#ifdef Q_OS_ANDROID
	notification->setIconName("kaidan-bw");
#endif
	notification->setDefaultAction("Open");
	notification->setActions(QStringList {
	    QObject::tr("Mark as read")
	});

	QObject::connect(notification, &KNotification::defaultActivated, [=] {
		emit Kaidan::instance()->openChatPageRequested(senderJid);
		emit Kaidan::instance()->raiseWindowRequested();
	});
	QObject::connect(notification, &KNotification::action1Activated, [=] {
		emit Kaidan::instance()->rosterModel()->updateItemRequested(senderJid, [=](RosterItem &item) {
			item.setUnreadMessages(0);
		});
	});

	notification->sendEvent();
}
#else
void Notifications::sendMessageNotification(const QString&, const QString&, const QString&)
{
}
#endif
