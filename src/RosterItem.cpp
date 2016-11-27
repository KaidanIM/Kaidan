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
