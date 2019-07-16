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

#include <QCameraInfo>
#include <QAbstractListModel>

class CameraInfo : public QCameraInfo {
	Q_GADGET

	Q_PROPERTY(bool isNull READ isNull CONSTANT)
	Q_PROPERTY(QString deviceName READ deviceName CONSTANT)
	Q_PROPERTY(QString description READ description CONSTANT)
	Q_PROPERTY(QCamera::Position position READ position CONSTANT)
	Q_PROPERTY(int orientation READ orientation CONSTANT)

public:
	using QCameraInfo::QCameraInfo;
	explicit CameraInfo(const QString &deviceName);
	CameraInfo() = default;
	CameraInfo(const QCameraInfo &other);
};

class CameraModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(int rowCount READ rowCount NOTIFY camerasChanged)
	Q_PROPERTY(QList<QCameraInfo> cameras READ cameras NOTIFY camerasChanged)
	Q_PROPERTY(CameraInfo defaultCamera READ defaultCamera NOTIFY camerasChanged)
	Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
	Q_PROPERTY(CameraInfo currentCamera READ currentCamera WRITE setCurrentCamera NOTIFY currentIndexChanged)

public:
	enum CustomRoles {
		IsNullRole = Qt::UserRole,
		DeviceNameRole,
		DescriptionRole,
		PositionRole,
		OrientationRole,
		CameraInfoRole
	};

	explicit CameraModel(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;

	QList<QCameraInfo> cameras() const;
	static CameraInfo defaultCamera();

	int currentIndex() const;
	void setCurrentIndex(int currentIndex);

	CameraInfo currentCamera() const;
	void setCurrentCamera(const CameraInfo &currentCamera);

	Q_INVOKABLE CameraInfo camera(int row) const;
	Q_INVOKABLE int indexOf(const QString &deviceName) const;

	static CameraInfo camera(const QString &deviceName);
	static CameraInfo camera(QCamera::Position position);

public slots:
	void refresh();

signals:
	void camerasChanged();
	void currentIndexChanged();

private:
	QList<QCameraInfo> m_cameras;
	int m_currentIndex = -1;
};

Q_DECLARE_METATYPE(CameraInfo)
