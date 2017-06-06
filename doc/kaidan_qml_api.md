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

### `kaidan.connectionState = bool`
Is true, when connected to a server.

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

### `kaidan.newLoginNeeded()`
Will return true, if there is no account data to use for connecting.

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


## MessageController

The message controller class is for sending and receiving messages.

### `kaidan.messageController.messageModel = MessageModel`
This is the model needed to display the messages from the SQLite database. It
can be used as model in ListViews.

### `kaidan.messageController.recipient = string`
The current recipient JID. When changed, a filter will be added to the message DB.

### `kaidan.messageController.sendMessage(recipient, message)`
This will send the `message` to the `recipient`.
* `recipient = string`: The bare JID of the recipient; for example:
"bob@jabber.aserver.org".
* `message = string`: The message to be sent.


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


## RosterController

The roster controller is for getting, editing and adding contacts.

### `kaidan.rosterController.rosterModel = RosterModel`
The model for displaying the roster contacts from the SQLite database. It can be
used in ListViews.

### `kaidan.rosterController.addContact(jid, name)`
This will send a new request to add a new contact to the roster.
* `jid = string`: The bare JID of the new contact.
* `name = string`: The preferred nickname of the contact in the roster, can be
empty ("").

### `kaidan.rosterController.removeContact(jid)`
This will send a request to remove the contact that has the given `jid`.
* `jid = string`: The bare JID of the contact to remove.


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


## VCard

A VCard is used to store profile information as names, emails, telephone number,
addresses and photos. Currently only one email, the names and a photo are
supported.

### `version = string`
A version string of the VCard.

### `fullName = string`
The full name.

### `familyName = string`
The family name.

### `givenName = string`
The given name.

### `middleName = string`
The middle name(s).

### `prefix = string`
A name prefix.

### `suffix = string`
A name suffix.

### `nickname = string`
The nickname.

### `preferredEMailAddress = string`
The preferred EMail address. (read-only)

### `photo = string`
A base64 encoded image file. (read-only)

### `photoType = string`
The type of the photo; for example: "image/png". (read-only)


## VCardController

The VCard controller is for getting the own and other's VCards with their contact
information.

### `kaidan.vCardController.currentJid = string`
The current bare JID of the current loaded VCard, on changed a new VCard will be
loaded into `currentVCard`.

### `kaidan.vCardController.currentVCard = VCard`
The current VCard of the currently selected JID in `currentJid`. See
[VCard](#vcard) for information about VCards.

### `kaidan.vCardController.ownVCard = VCard`
The own VCard; see [VCard](#vcard). It's empty until the client has connected
and the response of the request is there.

###### Copyright (C) 2017 LNJ <<git@lnj.li>>; GNU General Public License, Version 3.0 or later
