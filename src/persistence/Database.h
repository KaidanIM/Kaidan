#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QSqlDatabase>

class Database : public QObject
{
	Q_OBJECT

public:
	explicit Database(QObject *parent = 0);
	bool isValid();
	QSqlDatabase* getPointer();

	void dumpDataToStdOut() const;

signals:

public slots:

private:
	bool databaseValid_;
	QSqlDatabase database_;
};

#endif // DATABASE_H
