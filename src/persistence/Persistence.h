#ifndef PERSISTENCE_H
#define PERSISTENCE_H

#include <QObject>

class Database;
class MessageController;

class Persistence : public QObject
{
	Q_OBJECT
	Q_PROPERTY(MessageController* messageController READ getMessageContoller NOTIFY messageContollerChanged)

public:
	explicit Persistence(QObject *parent = 0);
	~Persistence();
	bool isValid();

signals:
	void messageContollerChanged();

public slots:
	void addMessage(QString const &jid, QString const &message, unsigned int direction);
	void setCurrentChatPartner(QString const &jid);

private:
	MessageController* getMessageContoller();

	Database *db_;
	MessageController *messageController_;
	bool persistenceValid_;
};

#endif // PERSISTENCE_H
