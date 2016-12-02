#include "RosterContoller.h"

#include <QQmlContext>

RosterController::RosterController(QObject *parent) : QObject(parent), client_(NULL), rosterList_()
{
}

void RosterController::requestRosterFromClient(Swift::Client *client)
{
	client_ = client;
	client_->requestRoster();

	Swift::GetRosterRequest::ref rosterRequest = Swift::GetRosterRequest::create(client->getIQRouter());
	rosterRequest->onResponse.connect(bind(&RosterController::handleRosterReceived, this, _2));
	rosterRequest->send();
}

void RosterController::handleRosterReceived(Swift::ErrorPayload::ref error)
{
	if (error)
	{
		std::cerr << "Error receiving roster. Continuing anyway.";
	}
	else
	{
		//std::cout << "handleRosterReceived!!!" << std::endl;
		Swift::XMPPRoster* roster = client_->getRoster();
		std::vector<Swift::XMPPRosterItem> rosterItems = roster->getItems();

		std::vector<Swift::XMPPRosterItem>::iterator it;
		//std::cout << "size: " << rosterItems.size() << std::endl;

		for(it = rosterItems.begin(); it < rosterItems.end(); it++ )
		{
#if 0
			std::cout << "jid: " << (*it).getJID().toString() <<
						 ", Name: " << (*it).getName() <<
						 ", Subscription: " << (*it).getSubscription() << std::endl;
#endif

			rosterList_.append(new RosterItem(QString::fromStdString((*it).getJID().toString()),
											  QString::fromStdString((*it).getName()),
											  (Subscription)(*it).getSubscription()));

			emit rosterListChanged();
		}
	}
}

QQmlListProperty<RosterItem> RosterController::getRosterList()
{
	return QQmlListProperty<RosterItem>(this, rosterList_);
}

