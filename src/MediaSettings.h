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

#include <QAudioEncoderSettings>
#include <QCoreApplication>
#include <QImageEncoderSettings>
#include <QVideoEncoderSettings>

#include "AudioDeviceModel.h"
#include "CameraModel.h"

class QSettings;

class MediaSettings
{
	Q_GADGET

	Q_PROPERTY(CameraInfo camera MEMBER camera)
	Q_PROPERTY(AudioDeviceInfo audioInputDevice MEMBER audioInputDevice)
	Q_PROPERTY(QString container MEMBER container)

public:
	MediaSettings() = default;
	explicit MediaSettings(const CameraInfo &camera, const AudioDeviceInfo &audioInputDevice);

	void loadSettings(QSettings *settings);
	void saveSettings(QSettings *settings);
	void dumpProperties(const QString &context) const;

	bool operator==(const MediaSettings &other) const;
	bool operator!=(const MediaSettings &other) const;

	CameraInfo camera;
	AudioDeviceInfo audioInputDevice;
	QString container;
};

class CommonEncoderSettings
{
	Q_GADGET
	Q_DECLARE_TR_FUNCTIONS(CommonEncoderSettings)

	Q_PROPERTY(QString codec MEMBER codec)
	Q_PROPERTY(CommonEncoderSettings::EncodingQuality quality MEMBER quality)
	Q_PROPERTY(QVariantMap options MEMBER options)

public:
	enum class EncodingQuality {
		VeryLowQuality = QMultimedia::EncodingQuality::VeryLowQuality,
		LowQuality = QMultimedia::EncodingQuality::LowQuality,
		NormalQuality = QMultimedia::EncodingQuality::NormalQuality,
		HighQuality = QMultimedia::EncodingQuality::HighQuality,
		VeryHighQuality = QMultimedia::EncodingQuality::VeryHighQuality
	};
	Q_ENUM(EncodingQuality)

	enum class EncodingMode {
		ConstantQualityEncoding = QMultimedia::EncodingMode::ConstantQualityEncoding,
		ConstantBitRateEncoding = QMultimedia::EncodingMode::ConstantBitRateEncoding,
		AverageBitRateEncoding = QMultimedia::EncodingMode::AverageBitRateEncoding,
		TwoPassEncoding = QMultimedia::EncodingMode::TwoPassEncoding
	};
	Q_ENUM(EncodingMode)

	explicit CommonEncoderSettings(const QString &codec,
		CommonEncoderSettings::EncodingQuality quality,
		const QVariantMap &options);
	virtual ~CommonEncoderSettings() = default;

	virtual void loadSettings(QSettings *settings);
	virtual void saveSettings(QSettings *settings);
	virtual void dumpProperties(const QString &context) const = 0;

	bool operator==(const CommonEncoderSettings &other) const;
	bool operator!=(const CommonEncoderSettings &other) const;

	static QString toString(CommonEncoderSettings::EncodingQuality quality);
	static QString toString(CommonEncoderSettings::EncodingMode mode);

	QString codec;
	CommonEncoderSettings::EncodingQuality quality;
	QVariantMap options;
};

class ImageEncoderSettings : public CommonEncoderSettings
{
	Q_GADGET

	Q_PROPERTY(QSize resolution MEMBER resolution)

public:
	explicit ImageEncoderSettings(const QImageEncoderSettings &settings = { });

	void loadSettings(QSettings *settings) override;
	void saveSettings(QSettings *settings) override;
	void dumpProperties(const QString &context) const override;

	bool operator==(const ImageEncoderSettings &other) const;
	bool operator!=(const ImageEncoderSettings &other) const;

	QImageEncoderSettings toQImageEncoderSettings() const;

	QSize resolution;
};

class AudioEncoderSettings : public CommonEncoderSettings
{
	Q_GADGET

	Q_PROPERTY(CommonEncoderSettings::EncodingMode mode MEMBER mode)
	Q_PROPERTY(int bitRate MEMBER bitRate)
	Q_PROPERTY(int sampleRate MEMBER sampleRate)
	Q_PROPERTY(int channelCount MEMBER channelCount)

public:
	explicit AudioEncoderSettings(const QAudioEncoderSettings &settings = { });

	void loadSettings(QSettings *settings) override;
	void saveSettings(QSettings *settings) override;
	void dumpProperties(const QString &context) const override;

	bool operator==(const AudioEncoderSettings &other) const;
	bool operator!=(const AudioEncoderSettings &other) const;

	QAudioEncoderSettings toQAudioEncoderSettings() const;

	CommonEncoderSettings::EncodingMode mode;
	int bitRate;
	int sampleRate;
	int channelCount;
};

class VideoEncoderSettings : public CommonEncoderSettings
{
	Q_GADGET

	Q_PROPERTY(CommonEncoderSettings::EncodingMode mode MEMBER mode)
	Q_PROPERTY(int bitRate MEMBER bitRate)
	Q_PROPERTY(qreal frameRate MEMBER frameRate)
	Q_PROPERTY(QSize resolution MEMBER resolution)

public:
	explicit VideoEncoderSettings(const QVideoEncoderSettings &settings = { });

	void loadSettings(QSettings *settings) override;
	void saveSettings(QSettings *settings) override;
	void dumpProperties(const QString &context) const override;

	bool operator==(const VideoEncoderSettings &other) const;
	bool operator!=(const VideoEncoderSettings &other) const;

	QVideoEncoderSettings toQVideoEncoderSettings() const;

	CommonEncoderSettings::EncodingMode mode;
	int bitRate;
	qreal frameRate;
	QSize resolution;
};

Q_DECLARE_METATYPE(MediaSettings)
Q_DECLARE_METATYPE(ImageEncoderSettings)
Q_DECLARE_METATYPE(AudioEncoderSettings)
Q_DECLARE_METATYPE(VideoEncoderSettings)
