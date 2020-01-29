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

#include "MediaRecorder.h"
#include "Kaidan.h"

#include <QUrl>
#include <QFile>

/*
 * NOTES: Codecs and containers supported list are available as soon as the object is created.
 * Resolutions, frame rates etc are populated once the objects become *ready*.
 */

#define ENABLE_DEBUG false

#define SETTING_USER_DEFAULT QStringLiteral("User Default")
#define SETTING_DEFAULT_CAMERA_DEVICE_NAME QStringLiteral("Camera Device Name")
#define SETTING_DEFAULT_AUDIO_INPUT_DEVICE_NAME QStringLiteral("Audio Input Device Name")

static void connectCamera(QCamera *camera, MediaRecorder *receiver) {
	QObject::connect(camera, &QCamera::statusChanged, receiver, &MediaRecorder::readyChanged);
}

static void connectImageCapturer(CameraImageCapture *capturer, MediaRecorder *receiver) {
	QObject::connect(capturer, &CameraImageCapture::availabilityChanged, receiver, &MediaRecorder::availabilityStatusChanged);
	QObject::connect(capturer, QOverload<int, QCameraImageCapture::Error, const QString&>::of(&CameraImageCapture::error), receiver, &MediaRecorder::errorChanged);
	QObject::connect(capturer, &CameraImageCapture::actualLocationChanged, receiver, &MediaRecorder::actualLocationChanged);
	QObject::connect(capturer, &CameraImageCapture::readyForCaptureChanged, receiver, &MediaRecorder::readyChanged);
}

template <typename T>
static void connectMediaRecorder(T *recorder, MediaRecorder *receiver) {
	QObject::connect(recorder, QOverload<QMultimedia::AvailabilityStatus>::of(&T::availabilityChanged), receiver, &MediaRecorder::availabilityStatusChanged);
	QObject::connect(recorder, &T::stateChanged, receiver, &MediaRecorder::stateChanged);
	QObject::connect(recorder, &T::statusChanged, receiver, &MediaRecorder::statusChanged);
	QObject::connect(recorder, QOverload<QMediaRecorder::Error>::of(&T::error), receiver, &MediaRecorder::errorChanged);
	QObject::connect(recorder, &T::actualLocationChanged, receiver, &MediaRecorder::actualLocationChanged);
	QObject::connect(recorder, &T::durationChanged, receiver, &MediaRecorder::durationChanged);
	QObject::connect(recorder, &T::mutedChanged, receiver, &MediaRecorder::mutedChanged);
	QObject::connect(recorder, &T::volumeChanged, receiver, &MediaRecorder::volumeChanged);
	QObject::connect(recorder, QOverload<QMultimedia::AvailabilityStatus>::of(&T::availabilityChanged), receiver, &MediaRecorder::readyChanged);
	QObject::connect(recorder, &T::statusChanged, receiver, &MediaRecorder::readyChanged);
}

template <>
void connectMediaRecorder<QAudioRecorder>(QAudioRecorder *recorder, MediaRecorder *receiver) {
	connectMediaRecorder(qobject_cast<QMediaRecorder *>(recorder), receiver);
}

template <typename F>
static QStringList buildCodecList(const QStringList &codecs, F toString,
	MediaRecorder *recorder, const QString &startsWith = QString()) {
	QStringList entries(codecs);

	if (!startsWith.isEmpty()) {
		entries.erase(std::remove_if(entries.begin(), entries.end(), [&startsWith](const QString &entry) {
			return !entry.startsWith(startsWith, Qt::CaseInsensitive);
		}), entries.end());
	}

	std::sort(entries.begin(), entries.end(), [&toString, recorder](const QString &left, const QString &right) {
		return toString(left, recorder).compare(toString(right, recorder), Qt::CaseInsensitive) < 0;
	});

	entries.prepend(QString());

	return entries;
}

static QList<QSize> buildResolutionList(const QList<QSize> &resolutions) {
	QList<QSize> entries(resolutions);

	std::sort(entries.begin(), entries.end(), [](const QSize &left, const QSize &right) {
		return left.width() == right.width()
			       ? left.height() < right.height()
			       : left.width() < right.width();
	});

	entries.prepend(QSize());

	return entries;
}

static QList<CommonEncoderSettings::EncodingQuality> buildQualityList() {
	const QList<CommonEncoderSettings::EncodingQuality> entries {
		CommonEncoderSettings::EncodingQuality::VeryLowQuality,
		CommonEncoderSettings::EncodingQuality::LowQuality,
		CommonEncoderSettings::EncodingQuality::NormalQuality,
		CommonEncoderSettings::EncodingQuality::HighQuality,
		CommonEncoderSettings::EncodingQuality::VeryHighQuality
	};

	return entries;
}

static QList<int> buildSampleRateList(const QList<int> &sampleRates) {
	QList<int> entries(sampleRates);

	if (entries.isEmpty()) {
		entries = {
			8000,
			16000,
			22050,
			32000,
			37800,
			44100,
			48000,
			96000,
			192000
		};
	}

	std::sort(entries.begin(), entries.end(), [](const int left, const int right) {
		return left < right;
	});

	entries.prepend(-1);

	return entries;
}

static QList<qreal> buildFrameRateList(const QList<qreal> &frameRates) {
	QList<qreal> entries(frameRates);

	std::sort(entries.begin(), entries.end(), [](const qreal left, const qreal right) {
		return left < right;
	});

	entries.prepend(0.0);

	return entries;
}

MediaRecorder::MediaRecorder(QObject *parent)
	: QObject(parent)
	  , m_cameraModel(new CameraModel(this))
	  , m_audioDeviceModel(new AudioDeviceModel(this))
	  , m_containerModel(new MediaSettingsContainerModel(this, this))
	  , m_imageCodecModel(new MediaSettingsImageCodecModel(this, this))
	  , m_imageResolutionModel(new MediaSettingsResolutionModel(this, this))
	  , m_imageQualityModel(new MediaSettingsQualityModel(this, this))
	  , m_audioCodecModel(new MediaSettingsAudioCodecModel(this, this))
	  , m_audioSampleRateModel(new MediaSettingsAudioSampleRateModel(this, this))
	  , m_audioQualityModel(new MediaSettingsQualityModel(this, this))
	  , m_videoCodecModel(new MediaSettingsVideoCodecModel(this, this))
	  , m_videoResolutionModel(new MediaSettingsResolutionModel(this, this))
	  , m_videoFrameRateModel(new MediaSettingsVideoFrameRateModel(this, this))
	  , m_videoQualityModel(new MediaSettingsQualityModel(this, this))
{
	connect(this, &MediaRecorder::readyChanged, this, [this]() {
		if (!isReady()) {
			if (m_type == MediaRecorder::Type::Invalid) {
				m_cameraModel->setCurrentIndex(-1);
				m_audioDeviceModel->setCurrentIndex(-1);
				m_containerModel->clear();

				m_imageCodecModel->clear();
				m_imageResolutionModel->clear();
				m_imageQualityModel->clear();

				m_audioCodecModel->clear();
				m_audioSampleRateModel->clear();
				m_audioQualityModel->clear();

				m_videoCodecModel->clear();
				m_videoResolutionModel->clear();
				m_videoFrameRateModel->clear();
				m_videoQualityModel->clear();
			}

			return;
		}

#if ENABLE_DEBUG
		qDebug("syncProperties Begin");
#endif

		switch (m_type) {
		case MediaRecorder::Type::Invalid: {
			Q_UNREACHABLE();
			break;
		}
		case MediaRecorder::Type::Image: {
			m_cameraModel->setCurrentCamera(m_mediaSettings.camera);
			m_imageCodecModel->setValuesAndCurrentValue(buildCodecList(m_imageCapturer->supportedImageCodecs(), imageEncoderCodec, this),
				m_imageEncoderSettings.codec);
			m_imageResolutionModel->setValuesAndCurrentValue(buildResolutionList(m_imageCapturer->supportedResolutions()),
				m_imageEncoderSettings.resolution);
			m_imageQualityModel->setValuesAndCurrentValue(buildQualityList(),
				m_imageEncoderSettings.quality);
			break;
		}
		case MediaRecorder::Type::Audio: {
			m_audioDeviceModel->setCurrentAudioDevice(m_mediaSettings.audioInputDevice);
			m_containerModel->setValuesAndCurrentValue(buildCodecList(m_audioRecorder->supportedContainers(), encoderContainer, this, QStringLiteral("audio")),
				m_mediaSettings.container);
			m_audioCodecModel->setValuesAndCurrentValue(buildCodecList(m_audioRecorder->supportedAudioCodecs(), audioEncoderCodec, this),
				m_audioEncoderSettings.codec);
			m_audioSampleRateModel->setValuesAndCurrentValue(buildSampleRateList(m_audioRecorder->supportedAudioSampleRates()),
				m_audioEncoderSettings.sampleRate);
			m_audioQualityModel->setValuesAndCurrentValue(buildQualityList(),
				m_audioEncoderSettings.quality);
			break;
		}
		case MediaRecorder::Type::Video: {
			m_cameraModel->setCurrentCamera(m_mediaSettings.camera);
			m_containerModel->setValuesAndCurrentValue(buildCodecList(m_videoRecorder->supportedContainers(), encoderContainer, this, QStringLiteral("video")),
				m_mediaSettings.container);
			m_audioCodecModel->setValuesAndCurrentValue(buildCodecList(m_videoRecorder->supportedAudioCodecs(), audioEncoderCodec, this),
				m_audioEncoderSettings.codec);
			m_audioSampleRateModel->setValuesAndCurrentValue(buildSampleRateList(m_videoRecorder->supportedAudioSampleRates()),
				m_audioEncoderSettings.sampleRate);
			m_audioQualityModel->setValuesAndCurrentValue(buildQualityList(),
				m_audioEncoderSettings.quality);
			m_videoCodecModel->setValuesAndCurrentValue(buildCodecList(m_videoRecorder->supportedVideoCodecs(), videoEncoderCodec, this),
				m_videoEncoderSettings.codec);
			m_videoResolutionModel->setValuesAndCurrentValue(buildResolutionList(m_videoRecorder->supportedResolutions()),
				m_videoEncoderSettings.resolution);
			m_videoFrameRateModel->setValuesAndCurrentValue(buildFrameRateList(m_videoRecorder->supportedFrameRates()),
				m_videoEncoderSettings.frameRate);
			m_videoQualityModel->setValuesAndCurrentValue(buildQualityList(),
				m_videoEncoderSettings.quality);
			break;
		}
		}

#if ENABLE_DEBUG
		qDebug("syncProperties End");
#endif
	});
}

MediaRecorder::~MediaRecorder()
{
	if (isAvailable() && !isReady()) {
		cancel();
	}
}

MediaRecorder::Type MediaRecorder::type() const
{
	return m_type;
}

void MediaRecorder::setType(MediaRecorder::Type type)
{
	if (m_type == type) {
		return;
	}

	setupRecorder(type);
}

QMediaObject *MediaRecorder::mediaObject() const
{
	switch (m_type) {
	case MediaRecorder::Type::Image:
	case MediaRecorder::Type::Video:
		return m_camera;
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Audio:
		break;
	}

	return nullptr;
}

QString MediaRecorder::currentSettingsKey() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		break;
	case MediaRecorder::Type::Image:
		Q_ASSERT(!m_mediaSettings.camera.isNull());
		return settingsKey(m_type, m_mediaSettings.camera.deviceName());
	case MediaRecorder::Type::Audio:
		Q_ASSERT(!m_mediaSettings.audioInputDevice.isNull());
		return settingsKey(m_type, m_mediaSettings.audioInputDevice.deviceName());
	case MediaRecorder::Type::Video:
		Q_ASSERT(!m_mediaSettings.camera.isNull());
		return settingsKey(m_type, m_mediaSettings.camera.deviceName());
	}

	return { };
}

MediaRecorder::AvailabilityStatus MediaRecorder::availabilityStatus() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		break;
	case MediaRecorder::Type::Image:
		return static_cast<MediaRecorder::AvailabilityStatus>(m_imageCapturer->availability());
	case MediaRecorder::Type::Audio:
		return static_cast<MediaRecorder::AvailabilityStatus>(m_audioRecorder->availability());
	case MediaRecorder::Type::Video:
		return static_cast<MediaRecorder::AvailabilityStatus>(m_videoRecorder->availability());
	}

	return MediaRecorder::AvailabilityStatus::ServiceMissing;
}

MediaRecorder::State MediaRecorder::state() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		break;
	case MediaRecorder::Type::Audio:
		return static_cast<MediaRecorder::State>(m_audioRecorder->state());
	case MediaRecorder::Type::Video:
		return static_cast<MediaRecorder::State>(m_videoRecorder->state());
	}

	return MediaRecorder::State::StoppedState;
}

MediaRecorder::Status MediaRecorder::status() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		break;
	case MediaRecorder::Type::Audio:
		return static_cast<MediaRecorder::Status>(m_audioRecorder->status());
	case MediaRecorder::Type::Video:
		return static_cast<MediaRecorder::Status>(m_videoRecorder->status());
	}

	return MediaRecorder::Status::UnavailableStatus;
}

bool MediaRecorder::isAvailable() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		break;
	case MediaRecorder::Type::Image:
		return m_imageCapturer->isAvailable();
	case MediaRecorder::Type::Audio:
		return m_audioRecorder->isAvailable();
	case MediaRecorder::Type::Video:
		return m_videoRecorder->isAvailable();
	}

	return false;
}

bool MediaRecorder::isReady() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		break;
	case MediaRecorder::Type::Image:
		return m_imageCapturer->isReadyForCapture();
	case MediaRecorder::Type::Audio:
		return isAvailable()
		       && status() >= MediaRecorder::Status::UnloadedStatus
		       && status() <= MediaRecorder::Status::LoadedStatus;
	case MediaRecorder::Type::Video:
		return isAvailable()
		       && m_camera->status() == QCamera::Status::ActiveStatus
		       && status() >= MediaRecorder::Status::UnloadedStatus
		       && status() <= MediaRecorder::Status::LoadedStatus;
	}

	return false;
}

MediaRecorder::Error MediaRecorder::error() const
{
	static const QMap<int, MediaRecorder::Error> capturerMapping = {
		{ QCameraImageCapture::Error::NoError, MediaRecorder::Error::NoError },
		{ QCameraImageCapture::Error::NotReadyError, MediaRecorder::Error::NotReadyError },
		{ QCameraImageCapture::Error::ResourceError, MediaRecorder::Error::ResourceError },
		{ QCameraImageCapture::Error::OutOfSpaceError, MediaRecorder::Error::OutOfSpaceError },
		{ QCameraImageCapture::Error::NotSupportedFeatureError, MediaRecorder::Error::NotSupportedFeatureError },
		{ QCameraImageCapture::Error::FormatError, MediaRecorder::Error::FormatError }
	};
	static const QMap<int, MediaRecorder::Error> recorderMapping = {
		{ QMediaRecorder::Error::NoError, MediaRecorder::Error::NoError },
		{ QMediaRecorder::Error::ResourceError, MediaRecorder::Error::ResourceError },
		{ QMediaRecorder::Error::FormatError, MediaRecorder::Error::FormatError },
		{ QMediaRecorder::Error::OutOfSpaceError, MediaRecorder::Error::OutOfSpaceError }
	};

	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		break;
	case MediaRecorder::Type::Image: {
		const auto it = capturerMapping.constFind(m_imageCapturer->error());
		Q_ASSERT(it != capturerMapping.constEnd());
		return it.value();
	}
	case MediaRecorder::Type::Audio: {
		const auto it = recorderMapping.constFind(m_audioRecorder->error());
		Q_ASSERT(it != recorderMapping.constEnd());
		return it.value();
	}
	case MediaRecorder::Type::Video: {
		const auto it = recorderMapping.constFind(m_videoRecorder->error());
		Q_ASSERT(it != recorderMapping.constEnd());
		return it.value();
	}
	}

	return MediaRecorder::Error::NoError;
}

QString MediaRecorder::errorString() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		break;
	case MediaRecorder::Type::Image:
		return m_imageCapturer->errorString();
	case MediaRecorder::Type::Audio:
		return m_audioRecorder->errorString();
	case MediaRecorder::Type::Video:
		return m_videoRecorder->errorString();
	}

	return { };
}

QUrl MediaRecorder::actualLocation() const
{
	static const auto urlExists = [](const QUrl &url) {
		if (url.isEmpty() || (!url.scheme().isEmpty() && !url.isLocalFile())) {
			return url;
		}

		const QUrl u(url.isLocalFile() ? url : QUrl::fromLocalFile(url.toString()));
		return QFile::exists(u.toLocalFile()) ? u : QUrl();
	};

	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		break;
	case MediaRecorder::Type::Image:
		return urlExists(m_imageCapturer->actualLocation());
	case MediaRecorder::Type::Audio:
		return urlExists(m_audioRecorder->actualLocation());
	case MediaRecorder::Type::Video:
		return urlExists(m_videoRecorder->actualLocation());
	}

	return { };
}

qint64 MediaRecorder::duration() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		break;
	case MediaRecorder::Type::Audio:
		return m_audioRecorder->duration();
	case MediaRecorder::Type::Video:
		return m_videoRecorder->duration();
	}

	return 0;
}

bool MediaRecorder::isMuted() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		break;
	case MediaRecorder::Type::Audio:
		return m_audioRecorder->isMuted();
	case MediaRecorder::Type::Video:
		return m_videoRecorder->isMuted();
	}

	return false;
}

void MediaRecorder::setMuted(bool muted)
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Audio:
		m_audioRecorder->setMuted(muted);
		break;
	case MediaRecorder::Type::Video:
		m_videoRecorder->setMuted(muted);
		break;
	}
}

qreal MediaRecorder::volume() const
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		break;
	case MediaRecorder::Type::Audio:
		return m_audioRecorder->volume();
	case MediaRecorder::Type::Video:
		return m_videoRecorder->volume();
	}

	return false;
}

void MediaRecorder::setVolume(qreal volume)
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Audio:
		m_audioRecorder->setVolume(volume);
		break;
	case MediaRecorder::Type::Video:
		m_videoRecorder->setVolume(volume);
		break;
	}
}

MediaSettings MediaRecorder::mediaSettings() const
{
	return m_mediaSettings;
}

void MediaRecorder::setMediaSettings(const MediaSettings &settings)
{
#if ENABLE_DEBUG
	qDebug(Q_FUNC_INFO);
#endif

	if (settings == m_mediaSettings) {
		return;
	}

	const auto oldSettings = m_mediaSettings;

#if ENABLE_DEBUG
	settings.dumpProperties(QStringLiteral("New"));
	oldSettings.dumpProperties(QStringLiteral("Old"));
#endif

	m_mediaSettings = settings;

	if (!m_initializing) {
		switch (m_type) {
		case MediaRecorder::Type::Invalid:
			Q_UNREACHABLE();
			break;
		case MediaRecorder::Type::Image:
			if (oldSettings.camera != settings.camera) {
				setupRecorder(m_type);
			}

			break;
		case MediaRecorder::Type::Audio:
			if (oldSettings.audioInputDevice != settings.audioInputDevice) {
				setupRecorder(m_type);
			} else {
				m_audioRecorder->setContainerFormat(m_mediaSettings.container);
			}

			break;
		case MediaRecorder::Type::Video:
			if (oldSettings.camera != settings.camera) {
				setupRecorder(m_type);
			}  else {
				m_videoRecorder->setContainerFormat(m_mediaSettings.container);
			}

			break;
		}

		emit mediaSettingsChanged();
	}
}

ImageEncoderSettings MediaRecorder::imageEncoderSettings() const
{
	return m_imageEncoderSettings;
}

void MediaRecorder::setImageEncoderSettings(const ImageEncoderSettings &settings)
{
#if ENABLE_DEBUG
	qDebug(Q_FUNC_INFO);
#endif

	switch (m_type) {
	case MediaRecorder::Type::Image: {
		if (settings != m_imageEncoderSettings) {
#if ENABLE_DEBUG
			settings.dumpProperties(QStringLiteral("New"));
			m_imageEncoderSettings.dumpProperties(QStringLiteral("Old"));
#endif

			m_imageEncoderSettings = settings;

			if (!m_initializing) {
				m_imageCapturer->setEncodingSettings(m_imageEncoderSettings.toQImageEncoderSettings());
				emit imageEncoderSettingsChanged();
			}
		}

		break;
	}
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Audio:
	case MediaRecorder::Type::Video:
		Q_UNREACHABLE();
		break;
	}
}

AudioEncoderSettings MediaRecorder::audioEncoderSettings() const
{
	return m_audioEncoderSettings;
}

void MediaRecorder::setAudioEncoderSettings(const AudioEncoderSettings &settings)
{
#if ENABLE_DEBUG
	qDebug(Q_FUNC_INFO);
#endif

	switch (m_type) {
	case MediaRecorder::Type::Audio:
	case MediaRecorder::Type::Video: {
		if (settings != m_audioEncoderSettings) {
#if ENABLE_DEBUG
			settings.dumpProperties(QStringLiteral("New"));
			m_audioEncoderSettings.dumpProperties(QStringLiteral("Old"));
#endif

			m_audioEncoderSettings = settings;

			if (!m_initializing) {
				if (m_audioRecorder) {
					m_audioRecorder->setAudioSettings(m_audioEncoderSettings.toQAudioEncoderSettings());
				} else if (m_videoRecorder) {
					m_videoRecorder->setAudioSettings(m_audioEncoderSettings.toQAudioEncoderSettings());
				}

				emit audioEncoderSettingsChanged();

				// Changing audio settings does not trigger ready changed.
				if (m_type == MediaRecorder::Type::Audio) {
					emit readyChanged();
				}
			}
		}

		break;
	}
	case MediaRecorder::Type::Image:
	case MediaRecorder::Type::Invalid:
		Q_UNREACHABLE();
		break;
	}
}

VideoEncoderSettings MediaRecorder::videoEncoderSettings() const
{
	return m_videoEncoderSettings;
}

void MediaRecorder::setVideoEncoderSettings(const VideoEncoderSettings &settings)
{
#if ENABLE_DEBUG
	qDebug(Q_FUNC_INFO);
#endif

	switch (m_type) {
	case MediaRecorder::Type::Video: {
		if (settings != m_videoEncoderSettings) {
#if ENABLE_DEBUG
			settings.dumpProperties(QStringLiteral("New"));
			m_videoEncoderSettings.dumpProperties(QStringLiteral("Old"));
#endif

			m_videoEncoderSettings = settings;

			if (!m_initializing) {
				m_videoRecorder->setVideoSettings(m_videoEncoderSettings.toQVideoEncoderSettings());
				emit videoEncoderSettingsChanged();
			}
		}

		break;
	}
	case MediaRecorder::Type::Image:
	case MediaRecorder::Type::Audio:
	case MediaRecorder::Type::Invalid:
		Q_UNREACHABLE();
		break;
	}
}

void MediaRecorder::resetSettingsToDefaults()
{
#if ENABLE_DEBUG
	qDebug(Q_FUNC_INFO);
#endif

	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Image:
		setMediaSettings(MediaSettings());
		setImageEncoderSettings(ImageEncoderSettings());
		break;
	case MediaRecorder::Type::Audio:
		setMediaSettings(MediaSettings());
		setAudioEncoderSettings(AudioEncoderSettings());
		break;
	case MediaRecorder::Type::Video:
		setMediaSettings(MediaSettings());
		setAudioEncoderSettings(AudioEncoderSettings());
		setVideoEncoderSettings(VideoEncoderSettings());
		break;
	}
}

void MediaRecorder::resetUserSettings()
{
	resetSettings(userDefaultCamera(), userDefaultAudioInput());
}

void MediaRecorder::saveUserSettings()
{
#if ENABLE_DEBUG
	qDebug(Q_FUNC_INFO);
#endif

	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Image: {
		QSettings &settings(*Kaidan::instance()->getSettings());

		settings.beginGroup(settingsKey(m_type, SETTING_USER_DEFAULT));
		settings.setValue(SETTING_DEFAULT_CAMERA_DEVICE_NAME, m_mediaSettings.camera.deviceName());
		settings.endGroup();

		if (!m_mediaSettings.camera.isNull() || !m_mediaSettings.audioInputDevice.isNull()) {
			settings.beginGroup(currentSettingsKey());
			m_mediaSettings.saveSettings(&settings);
			m_imageEncoderSettings.saveSettings(&settings);
			settings.endGroup();
		}
		break;
	}
	case MediaRecorder::Type::Audio: {
		QSettings &settings(*Kaidan::instance()->getSettings());

		settings.beginGroup(settingsKey(m_type, SETTING_USER_DEFAULT));
		settings.setValue(SETTING_DEFAULT_AUDIO_INPUT_DEVICE_NAME, m_mediaSettings.audioInputDevice.deviceName());
		settings.endGroup();

		settings.beginGroup(currentSettingsKey());
		m_mediaSettings.saveSettings(&settings);
		m_audioEncoderSettings.saveSettings(&settings);
		settings.endGroup();
		break;
	}
	case MediaRecorder::Type::Video: {
		QSettings &settings(*Kaidan::instance()->getSettings());

		settings.beginGroup(settingsKey(m_type, SETTING_USER_DEFAULT));
		settings.setValue(SETTING_DEFAULT_CAMERA_DEVICE_NAME, m_mediaSettings.camera.deviceName());
		settings.endGroup();

		settings.beginGroup(currentSettingsKey());
		m_mediaSettings.saveSettings(&settings);
		m_audioEncoderSettings.saveSettings(&settings);
		m_videoEncoderSettings.saveSettings(&settings);
		settings.endGroup();
		break;
	}
	}
}

void MediaRecorder::record()
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Image:
#if ENABLE_DEBUG
		m_mediaSettings.dumpProperties("Capture");
		m_imageEncoderSettings.dumpProperties("Capture");
#endif
		m_imageCapturer->capture();
		break;
	case MediaRecorder::Type::Audio:
#if ENABLE_DEBUG
		m_mediaSettings.dumpProperties("Capture");
		m_audioEncoderSettings.dumpProperties("Capture");
#endif
		m_audioRecorder->record();
		break;
	case MediaRecorder::Type::Video:
#if ENABLE_DEBUG
		m_mediaSettings.dumpProperties("Capture");
		m_audioEncoderSettings.dumpProperties("Capture");
		m_videoEncoderSettings.dumpProperties("Capture");
#endif
		m_videoRecorder->record();
		break;
	}
}

void MediaRecorder::pause()
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Audio:
		m_audioRecorder->pause();
		break;
	case MediaRecorder::Type::Video:
		m_videoRecorder->pause();
		break;
	}
}

void MediaRecorder::stop()
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Image:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Audio:
		m_audioRecorder->stop();
		break;
	case MediaRecorder::Type::Video:
		m_videoRecorder->stop();
		break;
	}
}

void MediaRecorder::cancel()
{
	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Image:
		m_imageCapturer->cancelCapture();
		deleteActualLocation();
		break;
	case MediaRecorder::Type::Audio:
	case MediaRecorder::Type::Video: {
		if (state() == MediaRecorder::State::RecordingState) {
			stop();
		}

		deleteActualLocation();
		break;
	}
	}
}

bool MediaRecorder::setMediaObject(QMediaObject *object)
{
	Q_UNUSED(object);
	return false;
}

QString MediaRecorder::settingsKey(MediaRecorder::Type type, const QString &deviceName) const
{
	Q_ASSERT(type != MediaRecorder::Type::Invalid);

	const QString deviceKey = QString(deviceName)
					  .replace(QLatin1Char('/'), QLatin1Char(' '))
					  .replace(QLatin1Char('\\'), QLatin1Char(' '))
					  .replace(QLatin1Char('('), QLatin1Char(' '))
					  .replace(QLatin1Char(')'), QLatin1Char(' '))
					  .replace(QLatin1Char('='), QLatin1Char(' '))
					  .replace(QLatin1Char(':'), QLatin1Char(' '))
					  .replace(QLatin1Char('\n'), QLatin1Char(' '))
					  .simplified();
	return QString::fromLatin1("Multimedia/%1/%2").arg(Enums::toString(type), deviceKey);
}

CameraInfo MediaRecorder::userDefaultCamera() const
{
	if (m_type == MediaRecorder::Type::Invalid) {
		return CameraInfo();
	}

	QSettings &settings(*Kaidan::instance()->getSettings());
	CameraInfo cameraInfo = m_cameraModel->defaultCamera();

	settings.beginGroup(settingsKey(m_type, SETTING_USER_DEFAULT));

	if (settings.contains(SETTING_DEFAULT_CAMERA_DEVICE_NAME)) {
		const CameraInfo userCamera(settings.value(SETTING_DEFAULT_CAMERA_DEVICE_NAME).toString());

		if (!userCamera.isNull()) {
			cameraInfo = userCamera;
		}
	}

	settings.endGroup();

	return cameraInfo;
}

AudioDeviceInfo MediaRecorder::userDefaultAudioInput() const
{
	if (m_type == MediaRecorder::Type::Invalid) {
		return AudioDeviceInfo();
	}

	QSettings &settings(*Kaidan::instance()->getSettings());
	AudioDeviceInfo audioInput = m_audioDeviceModel->defaultAudioInputDevice();

	settings.beginGroup(settingsKey(m_type, SETTING_USER_DEFAULT));

	if (settings.contains(SETTING_DEFAULT_AUDIO_INPUT_DEVICE_NAME)) {
		const AudioDeviceInfo userAudioInput(AudioDeviceModel::audioInputDevice(settings.value(SETTING_DEFAULT_AUDIO_INPUT_DEVICE_NAME).toString()));

		if (!userAudioInput.isNull()) {
			audioInput = userAudioInput;
		}
	}

	settings.endGroup();

	return audioInput;
}

void MediaRecorder::resetSettings(const CameraInfo &camera, const AudioDeviceInfo &audioInput)
{
#if ENABLE_DEBUG
	qDebug(Q_FUNC_INFO);
#endif

	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		m_mediaSettings = MediaSettings();
		m_imageEncoderSettings = ImageEncoderSettings();
		m_audioEncoderSettings = AudioEncoderSettings();
		m_videoEncoderSettings = VideoEncoderSettings();
		break;
	case MediaRecorder::Type::Image: {
		QSettings &settings(*Kaidan::instance()->getSettings());
		MediaSettings mediaSettings(camera, AudioDeviceInfo());
		ImageEncoderSettings imageSettings;

		settings.beginGroup(settingsKey(m_type, mediaSettings.camera.deviceName()));
		mediaSettings.loadSettings(&settings);
		imageSettings.loadSettings(&settings);
		settings.endGroup();

		setMediaSettings(mediaSettings);
		setImageEncoderSettings(imageSettings);
		break;
	}
	case MediaRecorder::Type::Audio: {
		QSettings &settings(*Kaidan::instance()->getSettings());
		MediaSettings mediaSettings(CameraInfo(), audioInput);
		AudioEncoderSettings audioSettings;

		settings.beginGroup(settingsKey(m_type, mediaSettings.audioInputDevice.deviceName()));
		mediaSettings.loadSettings(&settings);
		audioSettings.loadSettings(&settings);
		settings.endGroup();

		setMediaSettings(mediaSettings);
		setAudioEncoderSettings(audioSettings);
		break;
	}
	case MediaRecorder::Type::Video: {
		QSettings &settings(*Kaidan::instance()->getSettings());
		MediaSettings mediaSettings(camera, AudioDeviceInfo());
		AudioEncoderSettings audioSettings;
		VideoEncoderSettings videoSettings;

		settings.beginGroup(settingsKey(m_type, mediaSettings.camera.deviceName()));
		mediaSettings.loadSettings(&settings);
		audioSettings.loadSettings(&settings);
		videoSettings.loadSettings(&settings);
		settings.endGroup();

		setMediaSettings(mediaSettings);
		setAudioEncoderSettings(audioSettings);
		setVideoEncoderSettings(videoSettings);
		break;
	}
	}
}

void MediaRecorder::setupCamera()
{
	// If there is no camera, there is no need to work any further
	if (m_mediaSettings.camera.isNull())
		return;

	m_camera = new QCamera(m_mediaSettings.camera, this);

	switch (m_type) {
	case MediaRecorder::Type::Invalid:
	case MediaRecorder::Type::Audio:
		Q_UNREACHABLE();
		break;
	case MediaRecorder::Type::Image:
		m_camera->setCaptureMode(QCamera::CaptureMode::CaptureStillImage);
		m_camera->imageProcessing()->setWhiteBalanceMode(QCameraImageProcessing::WhiteBalanceMode::WhiteBalanceFlash);
		m_camera->exposure()->setExposureCompensation(-1.0);
		m_camera->exposure()->setExposureMode(QCameraExposure::ExposureMode::ExposurePortrait);
		m_camera->exposure()->setFlashMode(QCameraExposure::FlashMode::FlashRedEyeReduction);
		break;
	case MediaRecorder::Type::Video:
		m_camera->setCaptureMode(QCamera::CaptureMode::CaptureVideo);
		m_camera->imageProcessing()->setWhiteBalanceMode(QCameraImageProcessing::WhiteBalanceMode::WhiteBalanceAuto);
		m_camera->exposure()->setExposureCompensation(0);
		m_camera->exposure()->setExposureMode(QCameraExposure::ExposureMode::ExposureAuto);
		m_camera->exposure()->setFlashMode(QCameraExposure::FlashMode::FlashOff);
		break;
	}

	connectCamera(m_camera, this);
}

void MediaRecorder::setupCapturer()
{
	if (m_camera)
		m_imageCapturer = new CameraImageCapture(m_camera, this);
	else
		m_imageCapturer = new CameraImageCapture({});

	m_imageCapturer->setEncodingSettings(m_imageEncoderSettings.toQImageEncoderSettings());

	connectImageCapturer(m_imageCapturer, this);
}

void MediaRecorder::setupAudioRecorder()
{
	m_audioRecorder = new QAudioRecorder(this);
	m_audioRecorder->setAudioInput(m_mediaSettings.audioInputDevice.deviceName());
	m_audioRecorder->setContainerFormat(m_mediaSettings.container);
	m_audioRecorder->setAudioSettings(m_audioEncoderSettings.toQAudioEncoderSettings());

	connectMediaRecorder(m_audioRecorder, this);
}

void MediaRecorder::setupVideoRecorder()
{
	if (m_camera)
		m_videoRecorder = new QMediaRecorder(m_camera, this);
	else
		m_videoRecorder = new QMediaRecorder({}, this);

	m_videoRecorder->setContainerFormat(m_mediaSettings.container);
	m_videoRecorder->setAudioSettings(m_audioEncoderSettings.toQAudioEncoderSettings());
	m_videoRecorder->setVideoSettings(m_videoEncoderSettings.toQVideoEncoderSettings());

	connectMediaRecorder(m_videoRecorder, this);
}

void MediaRecorder::setupRecorder(MediaRecorder::Type type)
{
	if (isAvailable() && !isReady()) {
		cancel();
	}

	delete m_imageCapturer; m_imageCapturer = nullptr;
	delete m_audioRecorder; m_audioRecorder = nullptr;
	delete m_videoRecorder; m_videoRecorder = nullptr;
	delete m_camera; m_camera = nullptr;

	if (m_type != type) {
		m_type = type;
		m_initializing = true;
		resetUserSettings();
		m_initializing = false;
	} else {
		m_initializing = true;
		resetSettings(m_mediaSettings.camera, m_mediaSettings.audioInputDevice);
		m_initializing = false;
	}

	switch (m_type) {
	case MediaRecorder::Type::Invalid:
		emit imageEncoderSettingsChanged();
		emit audioEncoderSettingsChanged();
		emit videoEncoderSettingsChanged();
		break;
	case MediaRecorder::Type::Image:
		setupCamera();
		setupCapturer();
		break;
	case MediaRecorder::Type::Audio:
		setupAudioRecorder();
		break;
	case MediaRecorder::Type::Video:
		setupCamera();
		setupVideoRecorder();
		break;
	}

	emit typeChanged();
	emit availabilityStatusChanged();
	emit stateChanged();
	emit statusChanged();
	emit readyChanged();
	emit errorChanged();
	emit actualLocationChanged();
	emit durationChanged();
	emit mutedChanged();
	emit volumeChanged();

	if (m_camera && m_camera->state() != QCamera::State::ActiveState) {
		m_camera->start();
	}
}

void MediaRecorder::deleteActualLocation()
{
	const QUrl url(actualLocation());

	if (!url.isEmpty() && url.isLocalFile()) {
		const QString filePath(url.toLocalFile());
		QFile file(filePath);

		if (file.exists()) {
			if (file.remove()) {
				emit actualLocationChanged();
			} else {
				qWarning("Can not delete record '%s'", qUtf8Printable(filePath));
			}
		}
	}
}

QString MediaRecorder::encoderContainer(const QString &container, const void *userData)
{
	const auto *recorder = static_cast<const MediaRecorder *>(userData);
	const auto *mediaRecorder = recorder->m_audioRecorder ? recorder->m_audioRecorder : recorder->m_videoRecorder;
	return container.isEmpty()
		       ? tr("Default")
		       : QString::fromLatin1("%1 (%2)").arg(mediaRecorder->containerDescription(container), container);
}

QString MediaRecorder::encoderResolution(const QSize &size, const void *userData)
{
	Q_UNUSED(userData);
	return size.isEmpty()
		       ? tr("Default")
		       : QString::fromLatin1("%1x%2").arg(QString::number(size.width()), QString::number(size.height()));
}

QString MediaRecorder::encoderQuality(const CommonEncoderSettings::EncodingQuality quality, const void *userData)
{
	Q_UNUSED(userData);
	return CommonEncoderSettings::toString(quality);
}

QString MediaRecorder::imageEncoderCodec(const QString &codec, const void *userData)
{
	const auto *recorder = static_cast<const MediaRecorder *>(userData);
	const auto *capturer = recorder->m_imageCapturer;
	return codec.isEmpty()
		       ? tr("Default")
		       : QString::fromLatin1("%1 (%2)").arg(capturer->imageCodecDescription(codec), codec);
}

QString MediaRecorder::audioEncoderCodec(const QString &codec, const void *userData)
{
	const auto *recorder = static_cast<const MediaRecorder *>(userData);
	const auto *mediaRecorder = recorder->m_audioRecorder ? recorder->m_audioRecorder : recorder->m_videoRecorder;
	return codec.isEmpty()
		       ? tr("Default")
		       : QString::fromLatin1("%1 (%2)").arg(mediaRecorder->audioCodecDescription(codec), codec);
}

QString MediaRecorder::audioEncoderSampleRate(const int sampleRate, const void *userData)
{
	Q_UNUSED(userData);
	return sampleRate == -1
		       ? tr("Default")
		       : QString::fromLatin1("%1 Hz").arg(sampleRate);
}

QString MediaRecorder::videoEncoderCodec(const QString &codec, const void *userData)
{
	const auto *recorder = static_cast<const MediaRecorder *>(userData);
	const auto *videoRecorder = recorder->m_videoRecorder;
	return codec.isEmpty()
		       ? tr("Default")
		       : QString::fromLatin1("%1 (%2)").arg(videoRecorder->videoCodecDescription(codec), codec);
}

QString MediaRecorder::videoEncoderFrameRate(const qreal frameRate, const void *userData)
{
	Q_UNUSED(userData);
	return qIsNull(frameRate)
		       ? tr("Default")
		       : QString::fromLatin1("%1 FPS").arg(frameRate);
}
