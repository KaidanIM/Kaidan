#ifndef ROSTERITEM_H
#define ROSTERITEM_H

#include <QObject>

enum Subscription
{
	None,
	To,
	From,
	Both,
	Remove
};

class RosterItem : public QObject
{
	Q_OBJECT

	Q_PROPERTY(QString name READ getName WRITE setName NOTIFY nameChanged)
	Q_PROPERTY(QString jid READ getJid WRITE setJid NOTIFY jidChanged)
	Q_PROPERTY(Subscription subscription READ getSubscription WRITE setSubscription NOTIFY subscriptionChanged)

public:
	explicit RosterItem(QObject *parent = 0);
	RosterItem(const QString& jid, const QString& name, const Subscription& subscription, QObject* parent = 0);

	QString getName();
	void setName(const QString& name);

	QString getJid();
	void setJid(const QString& jid);

	Subscription getSubscription();
	void setSubscription(const Subscription& subscription);

signals:
	void nameChanged();
	void jidChanged();
	void subscriptionChanged();

public slots:

private:
	QString jid_;
	QString name_;
	Subscription subscription_;
};

#endif // ROSTERITEM_H
