/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
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

#include "DataFormModel.h"
// Qt
#include <QUrl>
// QXmpp
#include <QXmppBitsOfBinaryContentId.h>
// Kaidan
#include "Globals.h"

DataFormModel::DataFormModel(QObject *parent)
	: QAbstractListModel(parent)
{
}

DataFormModel::DataFormModel(const QXmppDataForm &dataForm, QObject *parent)
	: QAbstractListModel(parent), m_form(dataForm)
{
}

DataFormModel::~DataFormModel() = default;

int DataFormModel::rowCount(const QModelIndex &parent) const
{
	// For list models only the root node (an invalid parent) should return the list's size. For all
	// other (valid) parents, rowCount() should return 0 so that it does not become a tree model.
	if (parent.isValid())
		return 0;

	return m_form.fields().size();
}

QVariant DataFormModel::data(const QModelIndex &index, int role) const
{
	if (!index.isValid() || !hasIndex(index.row(), index.column(), index.parent()))
		return {};

	const QXmppDataForm::Field &field = m_form.fields().at(index.row());

	switch(role) {
	case Key:
		return field.key();
	case Type:
		return field.type();
	case Label:
		return field.label();
	case IsRequired:
		return field.isRequired();
	case Value:
		return field.value();
	case Description:
		return field.description();
	case MediaUrl:
		return mediaSourceUri(field);
	}

	return {};
}

bool DataFormModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
	if (!index.isValid() || !hasIndex(index.row(), index.column(), index.parent()))
		return false;

	QXmppDataForm::Field field = m_form.fields().at(index.row());

	switch(role) {
	case Key:
		field.setKey(value.toString());
		break;
	case Type:
		field.setType(static_cast<QXmppDataForm::Field::Type>(value.toInt()));
		break;
	case Label:
		field.setLabel(value.toString());
		break;
	case IsRequired:
		field.setRequired(value.toBool());
		break;
	case Value:
		field.setValue(value);
		break;
	case Description:
		field.setDescription(value.toString());
		break;
	default:
		return false;
	}

	m_form.fields()[index.row()] = field;
	emit dataChanged(index, index, QVector<int>() << role);
	return true;
}

QHash<int, QByteArray> DataFormModel::roleNames() const
{
	return {
		{Key, QByteArrayLiteral("key")},
		{Type, QByteArrayLiteral("type")},
		{Label, QByteArrayLiteral("label")},
		{IsRequired, QByteArrayLiteral("isRequired")},
		{Value, QByteArrayLiteral("value")},
		{Description, QByteArrayLiteral("description")},
		{MediaUrl, QByteArrayLiteral("mediaUrl")}
	};
}

QXmppDataForm DataFormModel::form() const
{
	return m_form;
}

void DataFormModel::setForm(const QXmppDataForm &form)
{
	beginResetModel();
	m_form = form;
	endResetModel();

	emit formChanged();
}

QString DataFormModel::instructions() const
{
	return m_form.instructions();
}

QString DataFormModel::mediaSourceUri(const QXmppDataForm::Field &field) const
{
	QString mediaSourceUri;
	const auto mediaSources = field.mediaSources();

	for (const auto &mediaSource : mediaSources) {
		mediaSourceUri = mediaSource.uri().toString();
		// Prefer Bits of Binary URIs.
		// In most cases, the data has been received already then.
		if (QXmppBitsOfBinaryContentId::isBitsOfBinaryContentId(mediaSourceUri, true))
			return QStringLiteral("image://%1/%2").arg(BITS_OF_BINARY_IMAGE_PROVIDER_NAME, mediaSourceUri);
	}

	return mediaSourceUri;
}

QString DataFormModel::title() const
{
	return m_form.title();
}
