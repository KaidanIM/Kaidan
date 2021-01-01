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

#include <QAudioDeviceInfo>
#include <QAbstractListModel>

class AudioDeviceInfo : public QAudioDeviceInfo {
	Q_GADGET

	Q_PROPERTY(bool isNull READ isNull CONSTANT)
	Q_PROPERTY(QString deviceName READ deviceName CONSTANT)
	Q_PROPERTY(QString description READ description CONSTANT)
	Q_PROPERTY(QStringList supportedCodecs READ supportedCodecs CONSTANT)
	Q_PROPERTY(QList<int> supportedSampleRates READ supportedSampleRates CONSTANT)
	Q_PROPERTY(QList<int> supportedChannelCounts READ supportedChannelCounts CONSTANT)
	Q_PROPERTY(QList<int> supportedSampleSizes READ supportedSampleSizes CONSTANT)

public:
	using QAudioDeviceInfo::QAudioDeviceInfo;
	AudioDeviceInfo(const QAudioDeviceInfo &other);
	AudioDeviceInfo() = default;

	QString description() const;

	static QString description(const QString &deviceName);
};

class AudioDeviceModel : public QAbstractListModel
{
	Q_OBJECT

	Q_PROPERTY(AudioDeviceModel::Mode mode READ mode WRITE setMode NOTIFY modeChanged)
	Q_PROPERTY(int rowCount READ rowCount NOTIFY audioDevicesChanged)
	Q_PROPERTY(QList<QAudioDeviceInfo> audioDevices READ audioDevices NOTIFY audioDevicesChanged)
	Q_PROPERTY(AudioDeviceInfo defaultAudioDevice READ defaultAudioDevice NOTIFY audioDevicesChanged)
	Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentIndexChanged)
	Q_PROPERTY(AudioDeviceInfo currentAudioDevice READ currentAudioDevice WRITE setCurrentAudioDevice NOTIFY currentIndexChanged)

public:
	enum Mode {
		AudioInput = QAudio::Mode::AudioInput,
		AudioOutput = QAudio::Mode::AudioOutput
	};
	Q_ENUM(Mode)

	enum CustomRoles {
		IsNullRole = Qt::UserRole,
		DeviceNameRole,
		DescriptionRole,
		SupportedCodecsRole,
		SupportedSampleRatesRole,
		SupportedChannelCountsRole,
		SupportedSampleSizesRole,
		AudioDeviceInfoRole
	};

	explicit AudioDeviceModel(QObject *parent = nullptr);

	int rowCount(const QModelIndex &parent = QModelIndex()) const override;
	QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
	QHash<int, QByteArray> roleNames() const override;

	AudioDeviceModel::Mode mode() const;
	void setMode(AudioDeviceModel::Mode mode);

	QList<QAudioDeviceInfo> audioDevices() const;
	AudioDeviceInfo defaultAudioDevice() const;

	int currentIndex() const;
	void setCurrentIndex(int currentIndex);

	AudioDeviceInfo currentAudioDevice() const;
	void setCurrentAudioDevice(const AudioDeviceInfo &currentAudioDevice);

	Q_INVOKABLE AudioDeviceInfo audioDevice(int row) const;
	Q_INVOKABLE int indexOf(const QString &deviceName) const;

	static AudioDeviceInfo defaultAudioInputDevice();
	static AudioDeviceInfo audioInputDevice(const QString &deviceName);

	static AudioDeviceInfo defaultAudioOutputDevice();
	static AudioDeviceInfo audioOutputDevice(const QString &deviceName);

public slots:
	void refresh();

signals:
	void modeChanged();
	void audioDevicesChanged();
	void currentIndexChanged();

private:
	AudioDeviceModel::Mode m_mode = AudioDeviceModel::Mode::AudioInput;
	QList<QAudioDeviceInfo> m_audioDevices;
	int m_currentIndex = -1;
};

Q_DECLARE_METATYPE(AudioDeviceInfo)
