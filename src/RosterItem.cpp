/*
 *  Kaidan - Cross platform XMPP client
 *
 *  Copyright (C) 2016 geobra <s.g.b@gmx.de>
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan. If not, see <http://www.gnu.org/licenses/>.
 */

#include "RosterItem.h"

RosterItem::RosterItem(QObject *parent) : QObject(parent), jid_(""), name_(""), subscription_(None)
{
}

RosterItem::RosterItem(const QString &jid, const QString &name, const Subscription &subscription, QObject* parent) :
	QObject(parent), jid_(jid), name_(name), subscription_(subscription)
{
}

//
// Name
//

QString RosterItem::getName()
{
	return name_;
}

void RosterItem::setName(const QString &name)
{
	name_ = name;

	// emit name changed signal
	emit nameChanged();
}

//
// JID
//

QString RosterItem::getJid()
{
	return jid_;
}

void RosterItem::setJid(const QString &jid)
{
	jid_ = jid;

	// emit jid changed signal
	emit jidChanged();
}

//
// Subscription
//

Subscription RosterItem::getSubscription()
{
	return subscription_;
}

void RosterItem::setSubscription(const Subscription &subscription)
{
	subscription_ = subscription;

	// emit subscription changed signal
	emit subscriptionChanged();
}
