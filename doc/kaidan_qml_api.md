# Kaidan QML API documentation

This is the documentation of all functions and classes accessible from the
QML-side of the XMPP client Kaidan. This can be helpful, if you want to write a
new GUI for it as [KaidanSF](https://github.com/kaidanim/kaidansf).

## Main QML library - `io.github.kaidanim`

Use this to import the main Kaidan QML-library:
```qml
import io.github.kaidanim 1.0
```

This will be needed to use all registered QML types of Kaidan.


## Kaidan

These are the global functions and variables of the main Kaidan class.

### `kaidan.addContact(jid, name)`
This will send a new request to add a new contact to the roster.
* `jid = string`: The bare JID of the new contact.
* `name = string`: The preferred nickname of the contact in the roster, can be
empty ("").

### `kaidan.avatarStorage = AvatarFileStorage`
An AvatarStorage object to load automatically updating avatars.

### `kaidan.connectionState = bool`
Is true, when connected to a server.

### `kaidan.chatPartner = string`
The current recipient JID. When changed, a filter will be added to the message DB.

### `kaidan.jid = string`
The JID to use for the connection. It will be saved and restored in the settings.

### `kaidan.jidResource = string`
The XMPP resource to use for the connection. It will be saved and restored in
the settings.

### `kaidan.password = string`
The password to use for the connection. It will be saved and restored in the settings.

### `kaidan.mainConnect()`
Connects to the XMPP server of the set account and initialises all subcontrollers.

### `kaidan.mainDisconnect()`
Disconnect from XMPP server.

### `kaidan.messageModel = MessageModel`
This is the model needed to display the messages from the SQLite database. It
can be used as model in ListViews.

### `kaidan.newLoginNeeded()`
Will return true, if there is no account data to use for connecting.

### `kaidan.removeContact(jid)`
This will send a request to remove the contact that has the given `jid`.
* `jid = string`: The bare JID of the contact to remove.

### `kaidan.removeNewLinesFromString(input)`
This will use the QString::simplify function to remove all unneeded spaces, new lines, etc.
Will return the simplified string.

### `kaidan.rosterModel = RosterModel`
The model for displaying the roster contacts from the SQLite database. It can be
used in ListViews.

### `kaidan.sendMessage(recipient, message)`
This will send the `message` to the `recipient`.
* `recipient = string`: The bare JID of the recipient; for example:
"bob@jabber.aserver.org".
* `message = string`: The message to be sent.

### `kaidan.getResourcePath(resourceName)`
Get an absolute path of a data file; returns a file:// path
* `resourceName = string`: the relative path in the data folder of Kaidan; for example:
"images/banner.png"

### `kaidan.getVersionString()`
Get the full version string of this Kaidan build; returns a string, for example
"0.1.0" or "0.2.0-dev"

### `kaidan.connectionStateConnected()`
Signal, emitted when Kaidan has connected to a XMPP server.

### `kaidan.connectionStateDisconnected()`
Signal, emitted when Kaidan was disconnected or a try to connect wasn't successful.


## AvatarFileStorage
This is a simple storage to cache avatars. They will be updated automatically, whenever
an avatar has been changed.

### `getHashOfJid(jid)`
Will return a SHA3-256 hex hash string or an empty string if no avatar set for this JID.

### `getAvatarUrl(jid)`
Will return a file:// url to the local file of the JID's avatar.


## MessageModel

This is a QSqlTableModel. The table contains the following rows:

### `author = string`
The JID of author of the message.

### `author_resource = string`
The JID resource of the author of the message.

### `recipient = string`
The JID of the recipient of the message.

### `recipient_resource = string`
The JID resource of the recipient of the message.

### `timestamp = string`
The timestamp when the message was sent.

### `message = string`
The message body.

### `id = string`
The message id.

### `isDelivered = bool`
Is true, if the a delivery confirmation has arrived from the recipient's client.


## RosterModel

This is a QSqlTableModel. The table contains the following rows:

### `jid = string`
The JID of the contact.

### `name = string`
The nick name for this contact.

### `lastExchanged = string`
The date on which the last time a message was exchanged. (You can use this for
sorting)

### `unreadMessages = int`
The number of unread messages from this contact.

###### Copyright (C) 2017 LNJ <<git@lnj.li>>; GNU General Public License, Version 3.0 or later
