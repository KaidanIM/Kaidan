#include "Persistence.h"
#include "Database.h"
#include "MessageController.h"

Persistence::Persistence(QObject *parent) : QObject(parent), persistenceValid_(true)
{
	db_ = new Database(this);
	if (! db_->isValid())
	{
		persistenceValid_ = false;
	}
	else
	{
		messageController_ = new MessageController(db_, this);
	}
}

Persistence::~Persistence()
{
	// db_ has this as parent and gets free'd implicit from this;
}

void Persistence::addMessage(QString const &jid, QString const &message, unsigned int direction)
{
	messageController_->addMessage(jid, message, direction);
	emit messageContollerChanged();
}

void Persistence::setCurrentChatPartner(QString const &jid)
{
	messageController_->setFilterOnJid(jid);
}

bool Persistence::isValid()
{
	return persistenceValid_;
}

MessageController* Persistence::getMessageContoller()
{
	return messageController_;
}
