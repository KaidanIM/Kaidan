#ifndef ROSTERCONTROLLER_H
#define ROSTERCONTROLLER_H

#include <Swiften/Swiften.h>
#include <QObject>

class RosterController : public QObject
{
    Q_OBJECT
public:
    RosterController(Swift::Client *client, QObject *parent = 0);

signals:

public slots:

private:
    //RosterController();
    void handleRosterReceived(Swift::ErrorPayload::ref error);

    Swift::Client* client_;

    QList<QObject*> rosterList_;

};

#endif // ROSTERCONTROLLER_H
