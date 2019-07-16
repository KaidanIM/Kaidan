/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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
#include <QList>

#include <functional>

template <typename T>
class MediaSettingModel : public QAbstractListModel
{
public:
	enum CustomRoles {
		ValueRole = Qt::UserRole,
		DescriptionRole
	};

	using ToString = std::function<QString(const T &, const void *userData)>;

	using QAbstractListModel::QAbstractListModel;

	explicit MediaSettingModel(MediaSettingModel::ToString toString,
		const void *userData = nullptr, QObject *parent = nullptr)
		: QAbstractListModel(parent)
		  , m_toString(toString)
		  , m_userData(userData) {
	}

	int rowCount(const QModelIndex &parent = QModelIndex()) const override {
		return parent == QModelIndex() ? m_values.count() : 0;
	}

	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override {
		if (hasIndex(index.row(), index.column(), index.parent())) {
			switch (role) {
			case MediaSettingModel::CustomRoles::ValueRole:
				return QVariant::fromValue(m_values[index.row()]);
			case MediaSettingModel::CustomRoles::DescriptionRole:
				return toString(m_values[index.row()]);
			}
		}

		return { };
	}

	QHash<int, QByteArray> roleNames() const override {
		static const QHash<int, QByteArray> roles {
			{ MediaSettingModel::CustomRoles::ValueRole, QByteArrayLiteral("value") },
			{ MediaSettingModel::CustomRoles::DescriptionRole, QByteArrayLiteral("description") }
		};

		return roles;
	}

	MediaSettingModel::ToString toString() const {
		return m_toString;
	}

	void setToString(MediaSettingModel::ToString toString) {
		m_toString = toString;

		emit toStringChanged();

		const int count = rowCount();

		if (count > 0) {
			emit dataChanged(index(0, 0), index(count -1, 0));
		}
	}

	const void *userData() const {
		return m_userData;
	}

	void setUserData(const void *userData) {
		if (m_userData == userData) {
			return;
		}

		m_userData = userData;

		emit userDataChanged();

		const int count = rowCount();

		if (count > 0) {
			emit dataChanged(index(0, 0), index(count -1, 0));
		}
	}

	QList<T> values() const {
		return m_values;
	}

	void setValues(const QList<T> &values) {
		if (m_values == values) {
			return;
		}

		const int newCurrentIndex = m_currentIndex != -1 ? values.indexOf(currentValue()) : -1;
		const bool curIdxChanged = newCurrentIndex != m_currentIndex;

		beginResetModel();
		m_values = values;
		m_currentIndex = newCurrentIndex;
		endResetModel();

		emit valuesChanged();

		if (curIdxChanged) {
			emit currentIndexChanged();
		}
	}

	int currentIndex() const {
		return m_currentIndex;
	}

	void setCurrentIndex(int currentIndex) {
		if (currentIndex < 0 || currentIndex >= m_values.count()
			|| m_currentIndex == currentIndex) {
			return;
		}

		m_currentIndex = currentIndex;
		emit currentIndexChanged();
	}

	T currentValue() const {
		return m_currentIndex >= 0 && m_currentIndex < m_values.count()
			       ? m_values[m_currentIndex]
			       : T();
	}

	void setCurrentValue(const T &currentValue) {
		setCurrentIndex(indexOf(currentValue));
	}

	QString currentDescription() const {
		return m_currentIndex >= 0 && m_currentIndex < m_values.count()
			       ? toString(currentValue())
			       : QString();
	}

	void setValuesAndCurrentIndex(const QList<T> &values, int currentIndex) {
		if (m_values == values && m_currentIndex == currentIndex) {
			return;
		}

		beginResetModel();
		m_values = values;
		m_currentIndex = currentIndex >= 0 && currentIndex < m_values.count()
					 ? currentIndex
					 : -1;
		endResetModel();

		emit valuesChanged();
		emit currentIndexChanged();
	}

	void setValuesAndCurrentValue(const QList<T> &values, const T &currentValue) {
		setValuesAndCurrentIndex(values, values.indexOf(currentValue));
	}

	// Invokables
	virtual void clear() {
		beginResetModel();
		m_currentIndex = -1;
		m_values.clear();
		endResetModel();

		emit valuesChanged();
		emit currentIndexChanged();
	}

	virtual int indexOf(const T &value) const {
		return m_values.indexOf(value);
	}

	virtual T value(int index) const {
		if (index < 0 || index >= m_values.count()) {
			return { };
		}

		return m_values[index];
	}

	virtual QString description(int index) const {
		if (index < 0 || index >= m_values.count()) {
			return { };
		}

		return toString(m_values[index]);
	}

	virtual QString toString(const T &value) const {
		if (m_toString) {
			return m_toString(value, m_userData);
		}

		return QVariant::fromValue(value).toString();
	}

	// Signals
	virtual void toStringChanged() = 0;
	virtual void userDataChanged() = 0;
	virtual void valuesChanged() = 0;
	virtual void currentIndexChanged() = 0;

private:
	MediaSettingModel::ToString m_toString;
	const void *m_userData = nullptr;
	int m_currentIndex = -1;
	QList<T> m_values;
};

#define DECL_MEDIA_SETTING_MODEL(NAME, TYPE, TO_STRING)										\
class MediaSettings##NAME##Model : public MediaSettingModel<TYPE> {								\
	Q_OBJECT														\
																\
	Q_PROPERTY(MediaSettings##NAME##Model::ToString toString READ toString WRITE setToString NOTIFY toStringChanged)	\
	Q_PROPERTY(const void *userData READ userData WRITE setUserData NOTIFY userDataChanged)					\
	Q_PROPERTY(QList<TYPE> values READ values WRITE setValues NOTIFY valuesChanged)						\
	Q_PROPERTY(int rowCount READ rowCount NOTIFY valuesChanged)								\
	Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)				\
	Q_PROPERTY(TYPE currentValue READ currentValue WRITE setCurrentValue NOTIFY currentIndexChanged)			\
	Q_PROPERTY(QString currentDescription READ currentDescription NOTIFY currentIndexChanged)				\
																\
	using MSMT = MediaSettingModel<TYPE>;											\
																\
public:																\
	using MSMT::MSMT;													\
	explicit MediaSettings##NAME##Model(const void *userData, QObject *parent = nullptr)					\
		: MSMT(TO_STRING, userData, parent)										\
	{ }															\
																\
	explicit MediaSettings##NAME##Model(QObject *parent = nullptr)								\
		: MSMT(parent)													\
	{ }															\
																\
	using MSMT::toString;													\
	Q_INVOKABLE void clear() override { MSMT::clear(); }									\
	Q_INVOKABLE int indexOf(const TYPE &value) const override { return MSMT::indexOf(value); }				\
	Q_INVOKABLE TYPE value(int index) const override { return MSMT::value(index); }						\
	Q_INVOKABLE QString description(int index) const override { return MSMT::description(index); }				\
	Q_INVOKABLE QString toString(const TYPE &value) const override { return MSMT::toString(value); }			\
																\
Q_SIGNALS:															\
	void toStringChanged() override;											\
	void userDataChanged() override;											\
	void valuesChanged() override;												\
	void currentIndexChanged() override;											\
}
