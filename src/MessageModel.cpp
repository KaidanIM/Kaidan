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

#include "MessageModel.h"

#include <chrono>

// Qt
#include <QGuiApplication>
#include <QTimer>
// QXmpp
#include <QXmppUtils.h>
// Kaidan
#include "AccountManager.h"
#include "Kaidan.h"
#include "MessageDb.h"
#include "MessageHandler.h"
#include "Notifications.h"
#include "QmlUtils.h"
#include "RosterModel.h"

using namespace std::chrono_literals;

constexpr auto PAUSED_TYPING_TIMEOUT = 10s;
constexpr auto ACTIVE_TIMEOUT = 2min;
constexpr auto TYPING_TIMEOUT = 2s;

// defines that the message is suitable for correction only if it is among the N latest messages
constexpr int MAX_CORRECTION_MESSAGE_COUNT_DEPTH = 20;
// defines that the message is suitable for correction only if it has ben sent not earlier than N days ago
constexpr int MAX_CORRECTION_MESSAGE_DAYS_DEPTH = 2;

MessageModel *MessageModel::s_instance = nullptr;

MessageModel *MessageModel::instance()
{
	return s_instance;
}

MessageModel::MessageModel(QObject *parent)
	: QAbstractListModel(parent),
	  m_composingTimer(new QTimer(this)),
	  m_stateTimeoutTimer(new QTimer(this)),
	  m_inactiveTimer(new QTimer(this)),
	  m_chatPartnerChatStateTimeout(new QTimer(this))
{
	Q_ASSERT(!s_instance);
	s_instance = this;

	// Timer to set state to paused
	m_composingTimer->setSingleShot(true);
	m_composingTimer->setInterval(TYPING_TIMEOUT);
	m_composingTimer->callOnTimeout(this, [this] {
		sendChatState(QXmppMessage::Paused);

		// 10 seconds after user stopped typing, remove "paused" state
		m_stateTimeoutTimer->start(PAUSED_TYPING_TIMEOUT);
	});

	// Timer to reset typing-related notifications like paused and composing to active
	m_stateTimeoutTimer->setSingleShot(true);
	m_stateTimeoutTimer->callOnTimeout(this, [this] {
		sendChatState(QXmppMessage::Active);
	});

	// Timer to time out active state
	m_inactiveTimer->setSingleShot(true);
	m_inactiveTimer->setInterval(ACTIVE_TIMEOUT);
	m_inactiveTimer->callOnTimeout(this, [this] {
		sendChatState(QXmppMessage::Inactive);
	});

	// Timer to reset the chat partners state
	// if they lost connection while a state other then gone was active
	m_chatPartnerChatStateTimeout->setSingleShot(true);
	m_chatPartnerChatStateTimeout->setInterval(ACTIVE_TIMEOUT);
	m_chatPartnerChatStateTimeout->callOnTimeout(this, [this] {
		m_chatPartnerChatState = QXmppMessage::Gone;
		m_chatStateCache.insert(m_currentChatJid, QXmppMessage::Gone);
		emit chatStateChanged();
	});

	connect(MessageDb::instance(), &MessageDb::messagesFetched,
	        this, &MessageModel::handleMessagesFetched);
	connect(MessageDb::instance(), &MessageDb::pendingMessagesFetched,
	        this, &MessageModel::pendingMessagesFetched);

	connect(this, &MessageModel::addMessageRequested,
	        this, &MessageModel::addMessage);
	connect(this, &MessageModel::addMessageRequested,
	        MessageDb::instance(), &MessageDb::addMessage);

	connect(this, &MessageModel::updateMessageRequested,
	        this, &MessageModel::updateMessage);
	connect(this, &MessageModel::updateMessageInDatabaseRequested,
	        MessageDb::instance(), &MessageDb::updateMessage);

	connect(this, &MessageModel::handleChatStateRequested,
		this, &MessageModel::handleChatState);

	connect(this, &MessageModel::removeMessagesRequested, this, &MessageModel::removeMessages);
	connect(this, &MessageModel::removeMessagesRequested, MessageDb::instance(), &MessageDb::removeMessages);
}

MessageModel::~MessageModel() = default;

bool MessageModel::isEmpty() const
{
	return m_messages.isEmpty();
}

int MessageModel::rowCount(const QModelIndex &) const
{
	return m_messages.length();
}

QHash<int, QByteArray> MessageModel::roleNames() const
{
	QHash<int, QByteArray> roles;
	roles[Timestamp] = "timestamp";
	roles[Id] = "id";
	roles[Sender] = "sender";
	roles[Recipient] = "recipient";
	roles[Body] = "body";
	roles[SentByMe] = "sentByMe";
	roles[MediaType] = "mediaType";
	roles[IsEdited] = "isEdited";
	roles[DeliveryState] = "deliveryState";
	roles[MediaUrl] = "mediaUrl";
	roles[MediaSize] = "mediaSize";
	roles[MediaContentType] = "mediaContentType";
	roles[MediaLastModified] = "mediaLastModifed";
	roles[MediaLocation] = "mediaLocation";
	roles[MediaThumb] = "mediaThumb";
	roles[IsSpoiler] = "isSpoiler";
	roles[SpoilerHint] = "spoilerHint";
	roles[ErrorText] = "errorText";
	roles[DeliveryStateIcon] = "deliveryStateIcon";
	roles[DeliveryStateName] = "deliveryStateName";
	return roles;
}

QVariant MessageModel::data(const QModelIndex &index, int role) const
{
	if (!hasIndex(index.row(), index.column(), index.parent())) {
		qWarning() << "Could not get data from message model." << index << role;
		return {};
	}
	Message msg = m_messages.at(index.row());

	switch (role) {
	case Timestamp:
		return msg.stamp();
	case Id:
		return msg.id();
	case Sender:
		return msg.from();
	case Recipient:
		return msg.to();
	case Body:
		return msg.body();
	case SentByMe:
		return msg.sentByMe();
	case MediaType:
		return QVariant::fromValue(msg.mediaType());
	case IsEdited:
		return msg.isEdited();
	case DeliveryState:
		return QVariant::fromValue(msg.deliveryState());
	case MediaUrl:
		return msg.outOfBandUrl();
	case MediaLocation:
		return msg.mediaLocation();
	case MediaContentType:
		return msg.mediaContentType();
	case MediaSize:
		return msg.mediaLastModified();
	case MediaLastModified:
		return msg.mediaLastModified();
	case IsSpoiler:
		return msg.isSpoiler();
	case SpoilerHint:
		return msg.spoilerHint();
	case ErrorText:
		return msg.errorText();
	case DeliveryStateIcon:
		switch (msg.deliveryState()) {
		case DeliveryState::Pending:
			return QmlUtils::getResourcePath("images/dots.svg");
		case DeliveryState::Sent:
			return QmlUtils::getResourcePath("images/check-mark-pale.svg");
		case DeliveryState::Delivered:
			return QmlUtils::getResourcePath("images/check-mark.svg");
		case DeliveryState::Error:
			return QmlUtils::getResourcePath("images/cross.svg");
		}
		return {};
	case DeliveryStateName:
		switch (msg.deliveryState()) {
		case DeliveryState::Pending:
			return tr("Pending");
		case DeliveryState::Sent:
			return tr("Sent");
		case DeliveryState::Delivered:
			return tr("Delivered");
		case DeliveryState::Error:
			return tr("Error");
		}
		return {};

	// TODO: add (only useful as soon as we have got SIMS)
	case MediaThumb:
		return {};
	}
	return {};
}

void MessageModel::fetchMore(const QModelIndex &)
{
	emit MessageDb::instance()->fetchMessagesRequested(
			AccountManager::instance()->jid(), m_currentChatJid, m_messages.size());
}

bool MessageModel::canFetchMore(const QModelIndex &) const
{
	return !m_fetchedAll;
}

QString MessageModel::currentAccountJid()
{
	return m_currentAccountJid;
}

QString MessageModel::currentChatJid()
{
	return m_currentChatJid;
}

void MessageModel::setCurrentChat(const QString &accountJid, const QString &chatJid)
{
	if (accountJid == m_currentAccountJid && chatJid == m_currentChatJid)
		return;

	// Send gone state to old chat partner
	sendChatState(QXmppMessage::State::Gone);

	m_currentAccountJid = accountJid;
	m_currentChatJid = chatJid;

	// Reset chat states
	m_ownChatState = QXmppMessage::State::None;
	m_chatPartnerChatState = m_chatStateCache.value(chatJid, QXmppMessage::State::Gone);
	m_composingTimer->stop();
	m_stateTimeoutTimer->stop();
	m_inactiveTimer->stop();
	m_chatPartnerChatStateTimeout->stop();

	// Send active state to new chat partner
	sendChatState(QXmppMessage::State::Active);

	emit currentChatJidChanged(chatJid);
	removeAllMessages();
}

bool MessageModel::isChatCurrentChat(const QString &accountJid, const QString &chatJid) const
{
	return accountJid == m_currentAccountJid && chatJid == m_currentChatJid;
}

void MessageModel::sendMessage(const QString &body, bool isSpoiler, const QString &spoilerHint)
{
	emit Kaidan::instance()->client()->messageHandler()->sendMessageRequested(
				currentChatJid(), body, isSpoiler, spoilerHint);

	m_composingTimer->stop();
	m_stateTimeoutTimer->stop();

	// Reset composing chat state after message is sent
	sendChatState(QXmppMessage::State::Active);
}

bool MessageModel::canCorrectMessage(int index) const
{
	// check index validity
	if (index < 0 || index >= m_messages.size())
		return false;

	// message needs to be sent by us and needs to be no error message
	const auto &msg = m_messages.at(index);
	if (!msg.sentByMe() || msg.deliveryState() == Enums::DeliveryState::Error)
		return false;

	// check time limit
	const auto timeThreshold =
		QDateTime::currentDateTimeUtc().addDays(-MAX_CORRECTION_MESSAGE_DAYS_DEPTH);
	if (msg.stamp() < timeThreshold)
		return false;

	// check messages count limit
	for (int i = 0, count = 0; i < index; i++) {
		if (m_messages.at(i).sentByMe() && ++count == MAX_CORRECTION_MESSAGE_COUNT_DEPTH)
			return false;
	}

	return true;
}

void MessageModel::handleMessagesFetched(const QVector<Message> &msgs)
{
	if (msgs.isEmpty())
		return;

	beginInsertRows(QModelIndex(), rowCount(), rowCount() + msgs.length() - 1);
	for (auto msg : msgs) {
		msg.setSentByMe(AccountManager::instance()->jid() == msg.from());
		processMessage(msg);
		m_messages << msg;
	}
	endInsertRows();

	if (msgs.length() < DB_MSG_QUERY_LIMIT)
		m_fetchedAll = true;
}

void MessageModel::removeMessages(const QString &accountJid, const QString &chatJid)
{
	if (accountJid == m_currentAccountJid && chatJid == m_currentChatJid)
		removeAllMessages();
}

void MessageModel::removeAllMessages()
{
	if (!m_messages.isEmpty()) {
		beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
		m_messages.clear();
		endRemoveRows();
	}

	m_fetchedAll = false;
}

void MessageModel::insertMessage(int idx, const Message &msg)
{
	beginInsertRows(QModelIndex(), idx, idx);
	m_messages.insert(idx, msg);
	endInsertRows();
}

void MessageModel::addMessage(Message msg)
{
	showMessageNotification(msg);

	if (QXmppUtils::jidToBareJid(msg.from()) == m_currentChatJid ||
			QXmppUtils::jidToBareJid(msg.to()) == m_currentChatJid) {
		processMessage(msg);

		// index where to add the new message
		int i = 0;
		for (const auto &message : qAsConst(m_messages)) {
			if (msg.stamp() > message.stamp()) {
				insertMessage(i, msg);
				return;
			}
			i++;
		}

		// add message to the end of the list
		insertMessage(i, msg);
	}
}

void MessageModel::updateMessage(const QString &id,
                                 const std::function<void(Message &)> &updateMsg)
{
	for (int i = 0; i < m_messages.length(); i++) {
		if (m_messages.at(i).id() == id) {
			// update message
			Message msg = m_messages.at(i);
			updateMsg(msg);

			// check if item was actually modified
			if (m_messages.at(i) == msg)
				return;

			// check, if the position of the new message may be different
			if (msg.stamp() == m_messages.at(i).stamp()) {
				beginRemoveRows(QModelIndex(), i, i);
				m_messages.removeAt(i);
				endRemoveRows();

				// add the message at the same position
				insertMessage(i, msg);
			} else {
				beginRemoveRows(QModelIndex(), i, i);
				m_messages.removeAt(i);
				endRemoveRows();

				// put to new position
				addMessage(msg);
			}

			showMessageNotification(msg);

			break;
		}
	}

	emit updateMessageInDatabaseRequested(id, updateMsg);
}

int MessageModel::searchForMessageFromNewToOld(const QString &searchString, const int startIndex) const
{
	int indexOfFoundMessage = startIndex;

	if (indexOfFoundMessage >= m_messages.size())
		indexOfFoundMessage = 0;

	for (; indexOfFoundMessage < m_messages.size(); indexOfFoundMessage++) {
		if (m_messages.at(indexOfFoundMessage).body().contains(searchString, Qt::CaseInsensitive))
			return indexOfFoundMessage;
	}

	return -1;
}

int MessageModel::searchForMessageFromOldToNew(const QString &searchString, const int startIndex) const
{
	int indexOfFoundMessage = startIndex;

	if (indexOfFoundMessage < 0)
		indexOfFoundMessage = m_messages.size() - 1;

	for (; indexOfFoundMessage >= 0; indexOfFoundMessage--) {
		if (m_messages.at(indexOfFoundMessage).body().contains(searchString, Qt::CaseInsensitive))
			break;
	}

	return indexOfFoundMessage;
}

void MessageModel::processMessage(Message &msg)
{
	if (msg.body().size() > MESSAGE_MAX_CHARS) {
		auto body = msg.body();
		body.truncate(MESSAGE_MAX_CHARS);
		msg.setBody(body);
	}
}

void MessageModel::sendPendingMessages()
{
	emit MessageDb::instance()->fetchPendingMessagesRequested(AccountManager::instance()->jid());
}

QXmppMessage::State MessageModel::chatState() const
{
	return m_chatPartnerChatState;
}

void MessageModel::sendChatState(QXmppMessage::State state)
{
	// Handle some special cases
	switch(QXmppMessage::State(state)) {
	case QXmppMessage::State::Composing:
		// Restart timer if new character was typed in
		m_composingTimer->start();
		break;
	case QXmppMessage::State::Active:
		// Start inactive timer when active was sent,
		// so we can set the state to inactive two minutes later
		m_inactiveTimer->start();
		m_composingTimer->stop();
		break;
	default:
		break;
	}

	// Only send if the state changed, filter duplicated
	if (state != m_ownChatState) {
		m_ownChatState = state;
		emit sendChatStateRequested(m_currentChatJid, state);
	}
}

void MessageModel::sendChatState(ChatState::State state)
{
	sendChatState(QXmppMessage::State(state));
}

void MessageModel::correctMessage(const QString &msgId, const QString &message)
{
	// Reset composing chat state
	m_composingTimer->stop();
	m_stateTimeoutTimer->stop();
	sendChatState(QXmppMessage::State::Active);

	const auto hasCorrectId = [&msgId](const Message& msg) {
		return msg.id() == msgId;
	};
	auto itr = std::find_if(m_messages.begin(), m_messages.end(), hasCorrectId);

	if (itr != m_messages.end()) {
		Message &msg = *itr;
		msg.setBody(message);
		if (msg.deliveryState() != Enums::DeliveryState::Pending) {
			msg.setId(QXmppUtils::generateStanzaHash());
			// Set replaceId only on first correction, so it's always the original id
			// (`id` is the id of the current edit, `replaceId` is the original id)
			if (!msg.isEdited()) {
				msg.setIsEdited(true);
				msg.setReplaceId(msgId);
			}
			msg.setDeliveryState(Enums::DeliveryState::Pending);

			if (ConnectionState(Kaidan::instance()->connectionState()) == Enums::ConnectionState::StateConnected) {
				// the trick with the time is important for the servers
				// this way they can tell which version of the message is the latest
				Message copy = msg;
				copy.setStamp(QDateTime::currentDateTimeUtc());
				emit sendCorrectedMessageRequested(copy);
			}
		} else if (!msg.isEdited()) {
			msg.setStamp(QDateTime::currentDateTimeUtc());
		}

		QModelIndex index = createIndex(std::distance(m_messages.begin(), itr), 0);
		emit dataChanged(index, index);

		emit updateMessageInDatabaseRequested(msgId, [=] (Message &localMessage) {
			localMessage = msg;
		});
	}
}

void MessageModel::handleChatState(const QString &bareJid, QXmppMessage::State state)
{
	m_chatStateCache[bareJid] = state;

	if (bareJid == m_currentChatJid) {
		m_chatPartnerChatState = state;
		m_chatPartnerChatStateTimeout->start();
		emit chatStateChanged();
	}
}

void MessageModel::showMessageNotification(const Message &message) const
{
	// Send a notification in the following cases:
	// * The message was not sent by the user from another resource and
	//   received via Message Carbons.
	// * Notifications from the chat partner are not muted.
	// * The corresponding chat is not opened while the application window
	//   is active.

	if (!message.sentByMe()) {
		const auto accountJid = AccountManager::instance()->jid();
		const auto chatJid = message.from();

		bool userMuted = !Kaidan::instance()->notificationsMuted(chatJid);
		bool chatActive =
				isChatCurrentChat(accountJid, chatJid) &&
				QGuiApplication::applicationState() != Qt::ApplicationActive;

		if (!userMuted && !chatActive) {
			const auto chatName = RosterModel::instance()->itemName(accountJid, chatJid);
			Notifications::sendMessageNotification(accountJid, chatJid, chatName, message.body());
		}
	}
}
