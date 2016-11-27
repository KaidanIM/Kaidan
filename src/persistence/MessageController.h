#ifndef MESSAGECONTROLLER_H
#define MESSAGECONTROLLER_H

#include <QSqlTableModel>

class Database;

class MessageController : public QSqlTableModel
{
	Q_OBJECT

public:
	explicit MessageController(QObject *parent = 0);
	MessageController(Database *db, QObject *parent = 0);

	Q_INVOKABLE QVariant data(const QModelIndex &requestedIndex, int role=Qt::DisplayRole ) const;
	virtual void setTable ( const QString &table_name );
	virtual QHash<int, QByteArray> roleNames() const;

	void setFilterOnJid(QString const &jidFiler);
	void addMessage(QString const &jid, QString const &message, unsigned int direction);

signals:
	void signalMessageReceived(QString);

public slots:

private:
	void generateRoleNames();

	QHash<int, QByteArray> roles_;
	Database *database_;
};

#endif // MESSAGECONTROLLER_H
