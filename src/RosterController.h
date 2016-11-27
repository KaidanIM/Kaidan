#ifndef ROSTERCONTROLLER_H
#define ROSTERCONTROLLER_H

// Qt
#include <QObject>
#include <QQmlListProperty>
// Swiften
#include <Swiften/Swiften.h>
// Kaidan
#include "RosterItem.h"


class RosterController : public QObject
{
	Q_OBJECT
	Q_PROPERTY(QQmlListProperty<RosterItem> rosterList READ getRosterList NOTIFY rosterListChanged)

public:
	RosterController(QObject *parent = 0);

	void requestRosterFromClient(Swift::Client *client);
	QQmlListProperty<RosterItem> getRosterList();

signals:
	void rosterListChanged();

public slots:

private:
	void handleRosterReceived(Swift::ErrorPayload::ref error);

	Swift::Client* client_;
	QList<RosterItem*> rosterList_;

};

#endif // ROSTERCONTROLLER_H
