/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2021 Kaidan developers and contributors
 *  (see the LICENSE file for a full list of copyright authors)
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  In addition, as a special exception, the author of Kaidan gives
 *  permission to link the code of its release with the OpenSSL
 *  project's "OpenSSL" library (or with modified versions of it that
 *  use the same license as the "OpenSSL" library), and distribute the
 *  linked executables. You must obey the GNU General Public License in
 *  all respects for all of the code used other than "OpenSSL". If you
 *  modify this file, you may extend this exception to your version of
 *  the file, but you are not obligated to do so.  If you do not wish to
 *  do so, delete this exception statement from your version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

// Qt
#include <QAbstractListModel>
// QXmpp
#include <QXmppMessage.h>
// Kaidan
#include "Message.h"

class QTimer;
class Kaidan;

class ChatState : public QObject
{
	Q_OBJECT

public:
	enum State {
		None = QXmppMessage::None,
		Active = QXmppMessage::Active,
		Inactive = QXmppMessage::Inactive,
		Gone = QXmppMessage::Gone,
		Composing = QXmppMessage::Composing,
		Paused = QXmppMessage::Paused
	};
	Q_ENUM(State)
};

Q_DECLARE_METATYPE(ChatState::State)

class MessageModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(QString currentAccountJid READ currentAccountJid NOTIFY currentAccountJidChanged)
	Q_PROPERTY(QString currentChatJid READ currentChatJid NOTIFY currentChatJidChanged)
	Q_PROPERTY(QXmppMessage::State chatState READ chatState NOTIFY chatStateChanged)

public:
	// Basically copy from QXmpp, but we need to expose this to QML
	enum MessageRoles {
		Timestamp = Qt::UserRole + 1,
		Id,
		Sender,
		Recipient,
		Body,
		SentByMe,
		MediaType,
		IsEdited,
		DeliveryState,
		MediaUrl,
		MediaSize,
		MediaContentType,
		MediaLastModified,
		MediaLocation,
		MediaThumb,
		IsSpoiler,
		SpoilerHint,
		ErrorText,
		DeliveryStateIcon,
		DeliveryStateName
	};
	Q_ENUM(MessageRoles)

	static MessageModel *instance();

	MessageModel(QObject *parent = nullptr);
	~MessageModel();

	Q_REQUIRED_RESULT bool isEmpty() const;
	Q_REQUIRED_RESULT int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	Q_REQUIRED_RESULT QHash<int, QByteArray> roleNames() const override;
	Q_REQUIRED_RESULT QVariant data(const QModelIndex &index, int role) const override;

	Q_INVOKABLE void fetchMore(const QModelIndex &parent) override;
	Q_INVOKABLE bool canFetchMore(const QModelIndex &parent) const override;

	QString currentAccountJid();
	QString currentChatJid();
	Q_INVOKABLE void setCurrentChat(const QString &accountJid, const QString &chatJid);

	/**
	 * Determines whether a chat is the currently open chat.
	 *
	 * @param accountJid JID of the chat's account
	 * @param chatJid JID of the chat
	 *
	 * @return true if the chat is currently open, otherwise false
	 */
	bool isChatCurrentChat(const QString &accountJid, const QString &chatJid) const;

	Q_INVOKABLE void sendMessage(const QString &body, bool isSpoiler, const QString &spoilerHint);

	Q_INVOKABLE bool canCorrectMessage(int index) const;

	/**
	 * Correct the last message
	 */
	Q_INVOKABLE void correctMessage(const QString &msgId, const QString &message);

	/**
	 * Searches from the most recent to the oldest message to find a given substring (case insensitive).
	 *
	 * If no index is passed, the search begins from the most recent message.
	 *
	 * @param searchString substring to search for
	 * @param startIndex index of the first message to search for the given string
	 *
	 * @return index of the first found message containing the given string or -1 if no message containing the given string could be found
	 */
	Q_INVOKABLE int searchForMessageFromNewToOld(const QString &searchString, const int startIndex = 0) const;

	/**
	 * Searches from the oldest to the most recent message to find a given substring (case insensitive).
	 *
	 * If no index is passed, the search begins from the oldest message.
	 *
	 * @param searchString substring to search for
	 * @param startIndex index of the first message to search for the given string
	 *
	 * @return index of the first found message containing the given string or -1 if no message containing the given string could be found
	 */
	Q_INVOKABLE int searchForMessageFromOldToNew(const QString &searchString, const int startIndex = -1) const;

	/**
	 * Sends pending messages again after searching them in the database.
	 */
	Q_INVOKABLE void sendPendingMessages();

	/**
	  * Returns the current chat state
	  */
	QXmppMessage::State chatState() const;

	/**
	  * Sends the chat state notification
	  */
	void sendChatState(QXmppMessage::State state);
	Q_INVOKABLE void sendChatState(ChatState::State state);

signals:
	void currentAccountJidChanged(const QString &accountJid);
	void currentChatJidChanged(const QString &currentChatJid);

	void addMessageRequested(const Message &msg);
	void updateMessageRequested(const QString &id,
	                            const std::function<void (Message &)> &updateMsg);
	void setMessageDeliveryStateRequested(const QString &msgId, Enums::DeliveryState state, const QString &errText = QString());
	void pendingMessagesFetched(const QVector<Message> &messages);
	void sendCorrectedMessageRequested(const Message &msg);
	void updateMessageInDatabaseRequested(const QString &id,
	                                      const std::function<void (Message &)> &updateMsg);
	void chatStateChanged();
	void sendChatStateRequested(const QString &bareJid, QXmppMessage::State state);
	void handleChatStateRequested(const QString &bareJid, QXmppMessage::State state);

	/**
	 * Emitted to remove all messages of an account or an account's chat.
	 *
	 * @param accountJid JID of the account whose messages are being removed
	 * @param chatJid JID of the chat whose messages are being removed (optional)
	 */
	void removeMessagesRequested(const QString &accountJid, const QString &chatJid = {});

private slots:
	void handleMessagesFetched(const QVector<Message> &m_messages);

	void addMessage(Message msg);
	void updateMessage(const QString &id,
	                   const std::function<void (Message &)> &updateMsg);

	void setMessageDeliveryState(const QString &msgId, Enums::DeliveryState state, const QString &errText = QString());

	void handleChatState(const QString &bareJid, QXmppMessage::State state);

private:
	/**
	 * Removes all messages of an account or an account's chat.
	 *
	 * @param accountJid JID of the account whose messages are being removed
	 * @param chatJid JID of the chat whose messages are being removed (optional)
	 */
	void removeMessages(const QString &accountJid, const QString &chatJid = {});

	void removeAllMessages();

	void insertMessage(int i, const Message &msg);

	/**
	 * Shortens messages to 10000 if longer to prevent DoS
	 * @param message to process
	 */
	void processMessage(Message &msg);

	QVector<Message> m_messages;
	QString m_currentAccountJid;
	QString m_currentChatJid;
	bool m_fetchedAll = false;

	QXmppMessage::State m_chatPartnerChatState = QXmppMessage::State::None;
	QXmppMessage::State m_ownChatState = QXmppMessage::State::None;
	QTimer *m_composingTimer;
	QTimer *m_stateTimeoutTimer;
	QTimer *m_inactiveTimer;
	QTimer *m_chatPartnerChatStateTimeout;
	QMap<QString, QXmppMessage::State> m_chatStateCache;

	static MessageModel *s_instance;
};
