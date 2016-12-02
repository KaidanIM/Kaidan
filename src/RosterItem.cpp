#include "RosterItem.h"

RosterItem::RosterItem(QObject *parent) : QObject(parent), jid_(""), name_(""), subscription_(None)
{
}

RosterItem::RosterItem(const QString &jid, const QString &name, const Subscription &subscription, QObject* parent) :
	QObject(parent), jid_(jid), name_(name), subscription_(subscription)
{
}

QString RosterItem::getName()
{
	return name_;
}

void RosterItem::setName(const QString &name)
{
	name_ = name;

	emit nameChanged();
}

QString RosterItem::getJid()
{
	return jid_;
}

void RosterItem::setJid(const QString &jid)
{
	jid_ = jid;

	emit jidChanged();
}

Subscription RosterItem::getSubscription()
{
	return subscription_;
}

void RosterItem::setSubscription(const Subscription &subscription)
{
	subscription_ = subscription;

	emit subscriptionChanged();
}
