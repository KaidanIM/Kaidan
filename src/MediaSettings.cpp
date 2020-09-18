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

#include "MediaSettings.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QSettings>

#define MEDIA_SETTINGS_GROUP QStringLiteral("Media Settings")
#define IMAGE_SETTINGS_GROUP QStringLiteral("Image Settings")
#define AUDIO_SETTINGS_GROUP QStringLiteral("Audio Settings")
#define VIDEO_SETTINGS_GROUP QStringLiteral("Video Settings")

#define SETTING_CONTAINER QStringLiteral("Container")
#define SETTING_ENCODING_QUALITY QStringLiteral("Quality")
#define SETTING_ENCODING_MODE QStringLiteral("Mode")
#define SETTING_ENCODING_OPTIONS QStringLiteral("Options")
#define SETTING_CODEC QStringLiteral("Codec")
#define SETTING_RESOLUTION QStringLiteral("Resolution")
#define SETTING_BIT_RATE QStringLiteral("Bit Rate")
#define SETTING_SAMPLE_RATE QStringLiteral("Sample Rate")
#define SETTING_FRAME_RATE QStringLiteral("Frame Rate")
#define SETTING_CHANNEL_COUNT QStringLiteral("Channel Count")

static QString toString(const QVariantMap &options) {
	if (options.isEmpty()) {
		return QString();
	}

	const QJsonDocument document(QJsonObject::fromVariantMap(options));
	return QString::fromUtf8(document.toJson(QJsonDocument::JsonFormat::Indented)).trimmed();
}

static QString toString(const QSize &size) {
	return QString::fromLatin1("%1x%2").arg(QString::number(size.width()),
		QString::number(size.height()));
}

MediaSettings::MediaSettings(const CameraInfo &camera, const AudioDeviceInfo &audioInputDevice)
	: camera(camera)
	  , audioInputDevice(audioInputDevice)
{
}

void MediaSettings::loadSettings(QSettings *settings)
{
	settings->beginGroup(MEDIA_SETTINGS_GROUP);

	if (settings->contains(SETTING_CONTAINER)) {
		container = settings->value(SETTING_CONTAINER).toString();
	}

	settings->endGroup();
}

void MediaSettings::saveSettings(QSettings *settings)
{
	settings->beginGroup(MEDIA_SETTINGS_GROUP);

	settings->setValue(SETTING_CONTAINER, container);

	settings->endGroup();
}

void MediaSettings::dumpProperties(const QString &context) const
{
	qDebug("%s - %s", Q_FUNC_INFO, qUtf8Printable(context));
	qDebug("Camera: %s", qUtf8Printable(camera.deviceName()));
	qDebug("Audio Input Device: %s", qUtf8Printable(audioInputDevice.deviceName()));
	qDebug("Container: %s", qUtf8Printable(container));
}

bool MediaSettings::operator==(const MediaSettings &other) const
{
	return camera == other.camera
	       && audioInputDevice == other.audioInputDevice
	       && container == other.container;
}

bool MediaSettings::operator!=(const MediaSettings &other) const
{
	return !operator==(other);
}

CommonEncoderSettings::CommonEncoderSettings(const QString &codec,
	CommonEncoderSettings::EncodingQuality quality,
	const QVariantMap &options)
	: codec(codec)
	  , quality(quality)
	  , options(options)
{
}

void CommonEncoderSettings::loadSettings(QSettings *settings)
{
	if (settings->contains(SETTING_CODEC)) {
		codec = settings->value(SETTING_CODEC).toString();
	}

	if (settings->contains(SETTING_ENCODING_QUALITY)) {
		const int value = settings->value(SETTING_ENCODING_QUALITY).toInt();
		quality = static_cast<CommonEncoderSettings::EncodingQuality>(value);
	}

	if (settings->contains(SETTING_ENCODING_OPTIONS)) {
		options = settings->value(SETTING_ENCODING_OPTIONS).toMap();
	}
}

void CommonEncoderSettings::saveSettings(QSettings *settings)
{
	settings->setValue(SETTING_CODEC, codec);
	settings->setValue(SETTING_ENCODING_QUALITY, static_cast<int>(quality));
	settings->setValue(SETTING_ENCODING_OPTIONS, options);
}

bool CommonEncoderSettings::operator==(const CommonEncoderSettings &other) const
{
	return codec == other.codec
	       && quality == other.quality
	       &&options == other.options;
}

bool CommonEncoderSettings::operator!=(const CommonEncoderSettings &other) const
{
	return !operator==(other);
}

QString CommonEncoderSettings::toString(CommonEncoderSettings::EncodingQuality quality)
{
	switch (quality) {
	case CommonEncoderSettings::EncodingQuality::VeryLowQuality:
		return tr("Very low");
	case CommonEncoderSettings::EncodingQuality::LowQuality:
		return tr("Low");
	case CommonEncoderSettings::EncodingQuality::NormalQuality:
		return tr("Normal");
	case CommonEncoderSettings::EncodingQuality::HighQuality:
		return tr("High");
	case CommonEncoderSettings::EncodingQuality::VeryHighQuality:
		return tr("Very high");
	}

	Q_UNREACHABLE();
	return { };
}

QString CommonEncoderSettings::toString(CommonEncoderSettings::EncodingMode mode)
{
	switch (mode) {
	case CommonEncoderSettings::EncodingMode::ConstantQualityEncoding:
		return tr("Constant quality");
	case CommonEncoderSettings::EncodingMode::ConstantBitRateEncoding:
		return tr("Constant bit rate");
	case CommonEncoderSettings::EncodingMode::AverageBitRateEncoding:
		return tr("Average bit rate");
	case CommonEncoderSettings::EncodingMode::TwoPassEncoding:
		return tr("Two pass");
	}

	Q_UNREACHABLE();
	return { };
}

ImageEncoderSettings::ImageEncoderSettings(const QImageEncoderSettings &settings)
	: CommonEncoderSettings(settings.codec(),
		  static_cast<CommonEncoderSettings::EncodingQuality>(settings.quality()),
		  settings.encodingOptions())
	  , resolution(settings.resolution())
{
}

void ImageEncoderSettings::loadSettings(QSettings *settings)
{
	settings->beginGroup(IMAGE_SETTINGS_GROUP);

	CommonEncoderSettings::loadSettings(settings);

	if (settings->contains(SETTING_RESOLUTION)) {
		resolution = settings->value(SETTING_RESOLUTION).toSize();
	}

	settings->endGroup();
}

void ImageEncoderSettings::saveSettings(QSettings *settings)
{
	settings->beginGroup(IMAGE_SETTINGS_GROUP);

	CommonEncoderSettings::saveSettings(settings);

	settings->setValue(SETTING_RESOLUTION, resolution);

	settings->endGroup();
}

void ImageEncoderSettings::dumpProperties(const QString &context) const
{
	qDebug("%s - %s", Q_FUNC_INFO, qUtf8Printable(context));
	qDebug("Codec: %s", qUtf8Printable(codec));
	qDebug("Quality: %s", qUtf8Printable(toString(quality)));
	qDebug("Options: %s", qUtf8Printable(::toString(options)));
	qDebug("Resolution: %s", qUtf8Printable(::toString(resolution)));
}

bool ImageEncoderSettings::operator==(const ImageEncoderSettings &other) const
{
	return CommonEncoderSettings::operator==(other)
	       && resolution == other.resolution;
}

bool ImageEncoderSettings::operator!=(const ImageEncoderSettings &other) const
{
	return !operator==(other);
}

QImageEncoderSettings ImageEncoderSettings::toQImageEncoderSettings() const
{
	QImageEncoderSettings settings;
	settings.setCodec(codec);
	settings.setQuality(static_cast<QMultimedia::EncodingQuality>(quality));
	settings.setEncodingOptions(options);
	settings.setResolution(resolution);
	return settings;
}

AudioEncoderSettings::AudioEncoderSettings(const QAudioEncoderSettings &settings)
	: CommonEncoderSettings(settings.codec(),
		  static_cast<CommonEncoderSettings::EncodingQuality>(settings.quality()),
		  settings.encodingOptions())
	  , mode(static_cast<CommonEncoderSettings::EncodingMode>(settings.encodingMode()))
	  , bitRate(settings.bitRate())
	  , sampleRate(settings.sampleRate())
	  , channelCount(settings.channelCount())
{
}

void AudioEncoderSettings::loadSettings(QSettings *settings)
{
	settings->beginGroup(AUDIO_SETTINGS_GROUP);

	CommonEncoderSettings::loadSettings(settings);

	if (settings->contains(SETTING_ENCODING_MODE)) {
		const int value = settings->value(SETTING_ENCODING_MODE).toInt();
		mode = static_cast<CommonEncoderSettings::EncodingMode>(value);
	}

	if (settings->contains(SETTING_BIT_RATE)) {
		bitRate = settings->value(SETTING_BIT_RATE).toInt();
	}

	if (settings->contains(SETTING_SAMPLE_RATE)) {
		sampleRate = settings->value(SETTING_SAMPLE_RATE).toInt();
	}

	if (settings->contains(SETTING_CHANNEL_COUNT)) {
		channelCount = settings->value(SETTING_CHANNEL_COUNT).toInt();
	}

	settings->endGroup();
}

void AudioEncoderSettings::saveSettings(QSettings *settings)
{
	settings->beginGroup(AUDIO_SETTINGS_GROUP);

	CommonEncoderSettings::saveSettings(settings);

	settings->setValue(SETTING_ENCODING_MODE, static_cast<int>(mode));
	settings->setValue(SETTING_BIT_RATE, bitRate);
	settings->setValue(SETTING_SAMPLE_RATE, sampleRate);
	settings->setValue(SETTING_CHANNEL_COUNT, channelCount);

	settings->endGroup();
}

void AudioEncoderSettings::dumpProperties(const QString &context) const
{
	qDebug("%s - %s", Q_FUNC_INFO, qUtf8Printable(context));
	qDebug("Codec: %s", qUtf8Printable(codec));
	qDebug("Quality: %s", qUtf8Printable(toString(quality)));
	qDebug("Options: %s", qUtf8Printable(::toString(options)));
	qDebug("Mode: %s", qUtf8Printable(toString(mode)));
	qDebug("BitRate: %i", bitRate);
	qDebug("SampleRate: %i", sampleRate);
	qDebug("ChannelCount: %i", channelCount);
}

bool AudioEncoderSettings::operator==(const AudioEncoderSettings &other) const
{
	return CommonEncoderSettings::operator==(other)
	       && mode == other.mode
	       && bitRate == other.bitRate
	       && sampleRate == other.sampleRate
	       && channelCount == other.channelCount;
}

bool AudioEncoderSettings::operator!=(const AudioEncoderSettings &other) const
{
	return !operator==(other);
}

QAudioEncoderSettings AudioEncoderSettings::toQAudioEncoderSettings() const
{
	QAudioEncoderSettings settings;
	settings.setCodec(codec);
	settings.setQuality(static_cast<QMultimedia::EncodingQuality>(quality));
	settings.setEncodingOptions(options);
	settings.setEncodingMode(static_cast<QMultimedia::EncodingMode>(mode));
	settings.setBitRate(bitRate);
	settings.setSampleRate(sampleRate);
	settings.setChannelCount(channelCount);
	return settings;
}

VideoEncoderSettings::VideoEncoderSettings(const QVideoEncoderSettings &settings)
	: CommonEncoderSettings(settings.codec(),
		  static_cast<CommonEncoderSettings::EncodingQuality>(settings.quality()),
		  settings.encodingOptions())
	  , mode(static_cast<CommonEncoderSettings::EncodingMode>(settings.encodingMode()))
	  , bitRate(settings.bitRate())
	  , frameRate(settings.frameRate())
	  , resolution(settings.resolution())
{
}

void VideoEncoderSettings::loadSettings(QSettings *settings)
{
	settings->beginGroup(VIDEO_SETTINGS_GROUP);

	CommonEncoderSettings::loadSettings(settings);

	if (settings->contains(SETTING_ENCODING_MODE)) {
		const int value = settings->value(SETTING_ENCODING_MODE).toInt();
		mode = static_cast<CommonEncoderSettings::EncodingMode>(value);
	}

	if (settings->contains(SETTING_BIT_RATE)) {
		bitRate = settings->value(SETTING_BIT_RATE).toInt();
	}

	if (settings->contains(SETTING_FRAME_RATE)) {
		frameRate = settings->value(SETTING_FRAME_RATE).toInt();
	}

	if (settings->contains(SETTING_RESOLUTION)) {
		resolution = settings->value(SETTING_RESOLUTION).toSize();
	}

	settings->endGroup();
}

void VideoEncoderSettings::saveSettings(QSettings *settings)
{
	settings->beginGroup(VIDEO_SETTINGS_GROUP);

	CommonEncoderSettings::saveSettings(settings);

	settings->setValue(SETTING_ENCODING_MODE, static_cast<int>(mode));
	settings->setValue(SETTING_BIT_RATE, bitRate);
	settings->setValue(SETTING_FRAME_RATE, frameRate);
	settings->setValue(SETTING_RESOLUTION, resolution);

	settings->endGroup();
}

void VideoEncoderSettings::dumpProperties(const QString &context) const
{
	qDebug("%s - %s", Q_FUNC_INFO, qUtf8Printable(context));
	qDebug("Codec: %s", qUtf8Printable(codec));
	qDebug("Quality: %s", qUtf8Printable(toString(quality)));
	qDebug("Options: %s", qUtf8Printable(::toString(options)));
	qDebug("Mode: %s", qUtf8Printable(toString(mode)));
	qDebug("BitRate: %i", bitRate);
	qDebug("FrameRate: %f", frameRate);
	qDebug("Resolution: %s", qUtf8Printable(::toString(resolution)));
}

bool VideoEncoderSettings::operator==(const VideoEncoderSettings &other) const
{
	return CommonEncoderSettings::operator==(other)
	       && mode == other.mode
	       && bitRate == other.bitRate
	       && frameRate == other.frameRate
	       && resolution == other.resolution;
}

bool VideoEncoderSettings::operator!=(const VideoEncoderSettings &other) const
{
	return !operator==(other);
}

QVideoEncoderSettings VideoEncoderSettings::toQVideoEncoderSettings() const
{
	QVideoEncoderSettings settings;
	settings.setCodec(codec);
	settings.setQuality(static_cast<QMultimedia::EncodingQuality>(quality));
	settings.setEncodingOptions(options);
	settings.setEncodingMode(static_cast<QMultimedia::EncodingMode>(mode));
	settings.setBitRate(bitRate);
	settings.setFrameRate(frameRate);
	settings.setResolution(resolution);
	return settings;
}
