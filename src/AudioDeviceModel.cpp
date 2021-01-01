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

#include "AudioDeviceModel.h"

static AudioDeviceInfo audioDeviceByDeviceName(QAudio::Mode mode, const QString &deviceName) {
	const auto audioDevices(QAudioDeviceInfo::availableDevices(mode));

	for (const auto &audioDevice: audioDevices) {
		if (audioDevice.deviceName() == deviceName) {
			return AudioDeviceInfo(audioDevice);
		}
	}

	return AudioDeviceInfo();
}

AudioDeviceInfo::AudioDeviceInfo(const QAudioDeviceInfo &other)
	: QAudioDeviceInfo(other)
{
}

QString AudioDeviceInfo::description() const
{
	return description(deviceName());
}

QString AudioDeviceInfo::description(const QString &deviceName)
{
	return QString(deviceName)
		.replace(QLatin1Char(':'), QLatin1Char(' '))
		.replace(QLatin1Char('='), QLatin1Char(' '))
		.replace(QLatin1Char(','), QLatin1Char(' '))
		.trimmed();
}

AudioDeviceModel::AudioDeviceModel(QObject *parent)
	: QAbstractListModel(parent)
{
	refresh();

	connect(this, &AudioDeviceModel::modeChanged, this, &AudioDeviceModel::refresh);
}

int AudioDeviceModel::rowCount(const QModelIndex &parent) const
{
	return parent == QModelIndex() ? m_audioDevices.count() : 0;
}

QVariant AudioDeviceModel::data(const QModelIndex &index, int role) const
{
	if (hasIndex(index.row(), index.column(), index.parent())) {
		const auto &audioDeviceInfo(m_audioDevices[index.row()]);

		switch (role) {
		case AudioDeviceModel::CustomRoles::IsNullRole:
			return audioDeviceInfo.isNull();
		case AudioDeviceModel::CustomRoles::DeviceNameRole:
			return audioDeviceInfo.deviceName();
		case AudioDeviceModel::CustomRoles::DescriptionRole:
			return AudioDeviceInfo::description(audioDeviceInfo.deviceName());
		case AudioDeviceModel::CustomRoles::SupportedCodecsRole:
			return audioDeviceInfo.supportedCodecs();
		case AudioDeviceModel::CustomRoles::SupportedSampleRatesRole:
			return QVariant::fromValue(audioDeviceInfo.supportedSampleRates());
		case AudioDeviceModel::CustomRoles::SupportedChannelCountsRole:
			return QVariant::fromValue(audioDeviceInfo.supportedChannelCounts());
		case AudioDeviceModel::CustomRoles::SupportedSampleSizesRole:
			return QVariant::fromValue(audioDeviceInfo.supportedSampleSizes());
		case AudioDeviceModel::CustomRoles::AudioDeviceInfoRole:
			return QVariant::fromValue(AudioDeviceInfo(audioDeviceInfo));
		}
	}

	return QVariant();
}

QHash<int, QByteArray> AudioDeviceModel::roleNames() const
{
	static const QHash<int, QByteArray> roles {
		{ IsNullRole, QByteArrayLiteral("isNull") },
		{ DeviceNameRole, QByteArrayLiteral("deviceName") },
		{ DescriptionRole, QByteArrayLiteral("description") },
		{ SupportedCodecsRole, QByteArrayLiteral("supportedCodecs") },
		{ SupportedSampleRatesRole, QByteArrayLiteral("supportedSampleRates") },
		{ SupportedChannelCountsRole, QByteArrayLiteral("supportedChannelCounts") },
		{ SupportedSampleSizesRole, QByteArrayLiteral("supportedSampleSizes") },
		{ AudioDeviceInfoRole, QByteArrayLiteral("audioDeviceInfo") }
	};

	return roles;
}

AudioDeviceModel::Mode AudioDeviceModel::mode() const
{
	return m_mode;
}

void AudioDeviceModel::setMode(AudioDeviceModel::Mode mode)
{
	if (m_mode == mode) {
		return;
	}

	m_mode = mode;
	emit modeChanged();
}

QList<QAudioDeviceInfo> AudioDeviceModel::audioDevices() const
{
	return m_audioDevices;
}

AudioDeviceInfo AudioDeviceModel::defaultAudioDevice() const
{
	switch (m_mode) {
	case AudioDeviceModel::Mode::AudioInput:
		return defaultAudioInputDevice();
	case AudioDeviceModel::Mode::AudioOutput:
		return defaultAudioOutputDevice();
	}

	Q_UNREACHABLE();
	return AudioDeviceInfo();
}

int AudioDeviceModel::currentIndex() const
{
	return m_currentIndex;
}

void AudioDeviceModel::setCurrentIndex(int currentIndex)
{
	if (currentIndex < 0 || currentIndex >= m_audioDevices.count()
		|| m_currentIndex == currentIndex) {
		return;
	}

	m_currentIndex = currentIndex;
	emit currentIndexChanged();
}

AudioDeviceInfo AudioDeviceModel::currentAudioDevice() const
{
	return m_currentIndex >= 0 && m_currentIndex < m_audioDevices.count()
		       ? AudioDeviceInfo(m_audioDevices[m_currentIndex])
		       : AudioDeviceInfo();
}

void AudioDeviceModel::setCurrentAudioDevice(const AudioDeviceInfo &currentAudioDevice)
{
	setCurrentIndex(m_audioDevices.indexOf(currentAudioDevice));
}

AudioDeviceInfo AudioDeviceModel::audioDevice(int row) const
{
	return hasIndex(row, 0)
		       ? AudioDeviceInfo(m_audioDevices[row])
		       : AudioDeviceInfo();
}

int AudioDeviceModel::indexOf(const QString &deviceName) const
{
	for (int i = 0; i < m_audioDevices.count(); ++i) {
		const auto &audioDevice(m_audioDevices[i]);

		if (audioDevice.deviceName() == deviceName) {
			return i;
		}
	}

	return -1;
}

AudioDeviceInfo AudioDeviceModel::defaultAudioInputDevice()
{
	return AudioDeviceInfo(QAudioDeviceInfo::defaultInputDevice());
}

AudioDeviceInfo AudioDeviceModel::audioInputDevice(const QString &deviceName)
{
	return audioDeviceByDeviceName(QAudio::AudioInput, deviceName);
}

AudioDeviceInfo AudioDeviceModel::defaultAudioOutputDevice()
{
	return AudioDeviceInfo(QAudioDeviceInfo::defaultOutputDevice());
}

AudioDeviceInfo AudioDeviceModel::audioOutputDevice(const QString &deviceName)
{
	return audioDeviceByDeviceName(QAudio::AudioOutput, deviceName);
}

void AudioDeviceModel::refresh()
{
	const auto audioDevices = QAudioDeviceInfo::availableDevices(static_cast<QAudio::Mode>(m_mode));

	if (m_audioDevices == audioDevices) {
		return;
	}

	beginResetModel();
	const QString currentDeviceName = currentAudioDevice().deviceName();
	const auto it = std::find_if(m_audioDevices.constBegin(), m_audioDevices.constEnd(),
		[&currentDeviceName](const QAudioDeviceInfo &deviceInfo) {
			return deviceInfo.deviceName() == currentDeviceName;
		});

	m_audioDevices = audioDevices;
	m_currentIndex = it == m_audioDevices.constEnd() ? -1 : it - m_audioDevices.constBegin();
	endResetModel();

	emit audioDevicesChanged();
	emit currentIndexChanged();
}
