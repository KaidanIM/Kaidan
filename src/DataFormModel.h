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

#include <QAbstractListModel>
#include <QHash>

#include <QXmppDataForm.h>

class DataFormModel : public QAbstractListModel
{
	Q_OBJECT
	Q_PROPERTY(QString title READ title NOTIFY formChanged)
	Q_PROPERTY(QString instructions READ instructions NOTIFY formChanged)

public:
	enum DataFormFieldType : quint8 {
		BooleanField,
		FixedField,
		HiddenField,
		JidMultiField,
		JidSingleField,
		ListMultiField,
		ListSingleField,
		TextMultiField,
		TextPrivateField,
		TextSingleField
	};
	Q_ENUM(DataFormFieldType)

	enum FormRoles {
		Key = Qt::UserRole + 1,
		Type,
		Label,
		IsRequired,
		Value,
		Description,
		MediaUrl
	};

	DataFormModel(QObject *parent = nullptr);
	DataFormModel(const QXmppDataForm &dataForm, QObject *parent = nullptr);
	~DataFormModel();

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
	QHash<int, QByteArray> roleNames() const override;

	QXmppDataForm form() const;
	void setForm(const QXmppDataForm &form);

	QString title() const;
	QString instructions() const;

signals:
	void formChanged();

protected:
	QXmppDataForm m_form;

private:
	QString mediaSourceUri(const QXmppDataForm::Field &field) const;
};
