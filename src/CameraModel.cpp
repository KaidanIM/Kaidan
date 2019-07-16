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

#include "CameraModel.h"

CameraInfo::CameraInfo(const QString &deviceName)
	: QCameraInfo(deviceName.toLocal8Bit())
{
}

CameraInfo::CameraInfo(const QCameraInfo &other)
	: QCameraInfo(other)
{
}

CameraModel::CameraModel(QObject *parent)
: QAbstractListModel(parent)
{
	refresh();
}

int CameraModel::rowCount(const QModelIndex &parent) const
{
	return parent == QModelIndex() ? m_cameras.count() : 0;
}

QVariant CameraModel::data(const QModelIndex &index, int role) const
{
	if (hasIndex(index.row(), index.column(), index.parent())) {
		const auto &cameraInfo(m_cameras[index.row()]);

		switch (role) {
		case CameraModel::CustomRoles::IsNullRole:
			return cameraInfo.isNull();
		case CameraModel::CustomRoles::DeviceNameRole:
			return cameraInfo.deviceName();
		case CameraModel::CustomRoles::DescriptionRole:
			return cameraInfo.description();
		case CameraModel::CustomRoles::PositionRole:
			return cameraInfo.position();
		case CameraModel::CustomRoles::OrientationRole:
			return cameraInfo.orientation();
		case CameraModel::CustomRoles::CameraInfoRole:
			return QVariant::fromValue(CameraInfo(cameraInfo));
		}
	}

	return QVariant();
}

QHash<int, QByteArray> CameraModel::roleNames() const
{
	static const QHash<int, QByteArray> roles {
		{ IsNullRole, QByteArrayLiteral("isNull") },
		{ DeviceNameRole, QByteArrayLiteral("deviceName") },
		{ DescriptionRole, QByteArrayLiteral("description") },
		{ PositionRole, QByteArrayLiteral("position") },
		{ OrientationRole, QByteArrayLiteral("orientation") },
		{ CameraInfoRole, QByteArrayLiteral("cameraInfo") }
	};

	return roles;
}

QList<QCameraInfo> CameraModel::cameras() const
{
	return m_cameras;
}

CameraInfo CameraModel::defaultCamera()
{
	return CameraInfo(QCameraInfo::defaultCamera());
}

int CameraModel::currentIndex() const
{
	return m_currentIndex;
}

void CameraModel::setCurrentIndex(int currentIndex)
{
	if (currentIndex < 0 || currentIndex >= m_cameras.count()
		|| m_currentIndex == currentIndex) {
		return;
	}

	m_currentIndex = currentIndex;
	emit currentIndexChanged();
}

CameraInfo CameraModel::currentCamera() const
{
	return m_currentIndex >= 0 && m_currentIndex < m_cameras.count()
		       ? CameraInfo(m_cameras[m_currentIndex])
		       : CameraInfo();
}

void CameraModel::setCurrentCamera(const CameraInfo &currentCamera)
{
	setCurrentIndex(m_cameras.indexOf(currentCamera));
}

CameraInfo CameraModel::camera(int row) const
{
	return hasIndex(row, 0)
		       ? CameraInfo(m_cameras[row])
		       : CameraInfo();
}

int CameraModel::indexOf(const QString &deviceName) const
{
	for (int i = 0; i < m_cameras.count(); ++i) {
		const auto &camera(m_cameras[i]);

		if (camera.deviceName() == deviceName) {
			return i;
		}
	}

	return -1;
}

CameraInfo CameraModel::camera(const QString &deviceName)
{
	return CameraInfo(deviceName);
}

CameraInfo CameraModel::camera(QCamera::Position position)
{
	const auto cameras = QCameraInfo::availableCameras(position);
	return cameras.isEmpty() ? CameraInfo() : CameraInfo(cameras.first());
}

void CameraModel::refresh()
{
	const auto cameras = QCameraInfo::availableCameras();

	if (m_cameras == cameras) {
		return;
	}

	beginResetModel();
	const QString currentDeviceName = currentCamera().deviceName();
	const auto it = std::find_if(m_cameras.constBegin(), m_cameras.constEnd(),
		[&currentDeviceName](const QCameraInfo &deviceInfo) {
			return deviceInfo.deviceName() == currentDeviceName;
		});

	m_cameras = cameras;
	m_currentIndex = it == m_cameras.constEnd() ? -1 : it - m_cameras.constBegin();
	endResetModel();

	emit camerasChanged();
	emit currentIndexChanged();
}
