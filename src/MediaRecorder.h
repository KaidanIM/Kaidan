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

#pragma once

#include "CameraModel.h"
#include "AudioDeviceModel.h"
#include "MediaSettings.h"
#include "MediaSettingModel.h"
#include "CameraImageCapture.h"

#include <QAudioRecorder>

class MediaSettingsContainerModel;
class MediaSettingsQualityModel;
class MediaSettingsResolutionModel;
class MediaSettingsImageCodecModel;
class MediaSettingsAudioCodecModel;
class MediaSettingsAudioSampleRateModel;
class MediaSettingsVideoCodecModel;
class MediaSettingsVideoFrameRateModel;

class MediaRecorder : public QObject, public QMediaBindableInterface
{
	friend MediaSettingsContainerModel;
	friend MediaSettingsQualityModel;
	friend MediaSettingsResolutionModel;
	friend MediaSettingsImageCodecModel;
	friend MediaSettingsAudioCodecModel;
	friend MediaSettingsAudioSampleRateModel;
	friend MediaSettingsVideoCodecModel;
	friend MediaSettingsVideoFrameRateModel;

	Q_OBJECT
	Q_INTERFACES(QMediaBindableInterface)

	Q_PROPERTY(MediaRecorder::Type type READ type WRITE setType NOTIFY typeChanged)
	Q_PROPERTY(QMediaObject *mediaObject READ mediaObject NOTIFY typeChanged)
	Q_PROPERTY(QString currentSettingsKey READ currentSettingsKey NOTIFY typeChanged)

	Q_PROPERTY(MediaRecorder::AvailabilityStatus availabilityStatus READ availabilityStatus NOTIFY availabilityStatusChanged)
	Q_PROPERTY(MediaRecorder::State state READ state NOTIFY stateChanged)
	Q_PROPERTY(MediaRecorder::Status status READ status NOTIFY statusChanged)
	Q_PROPERTY(bool isAvailable READ isAvailable NOTIFY availabilityStatusChanged)
	Q_PROPERTY(bool isReady READ isReady NOTIFY readyChanged)
	Q_PROPERTY(MediaRecorder::Error error READ error NOTIFY errorChanged)
	Q_PROPERTY(QString errorString READ errorString NOTIFY errorChanged)
	Q_PROPERTY(QUrl actualLocation READ actualLocation NOTIFY actualLocationChanged)
	Q_PROPERTY(qint64 duration READ duration NOTIFY durationChanged)

	Q_PROPERTY(bool muted READ isMuted WRITE setMuted NOTIFY mutedChanged)
	Q_PROPERTY(qreal volume READ volume WRITE setVolume NOTIFY volumeChanged)

	Q_PROPERTY(MediaSettings mediaSettings READ mediaSettings WRITE setMediaSettings NOTIFY mediaSettingsChanged)
	Q_PROPERTY(CameraModel *cameraModel MEMBER m_cameraModel CONSTANT)
	Q_PROPERTY(AudioDeviceModel *audioDeviceModel MEMBER m_audioDeviceModel CONSTANT)
	Q_PROPERTY(MediaSettingsContainerModel *containerModel MEMBER m_containerModel CONSTANT)

	Q_PROPERTY(ImageEncoderSettings imageEncoderSettings READ imageEncoderSettings WRITE setImageEncoderSettings NOTIFY imageEncoderSettingsChanged)
	Q_PROPERTY(MediaSettingsImageCodecModel *imageCodecModel MEMBER m_imageCodecModel CONSTANT)
	Q_PROPERTY(MediaSettingsResolutionModel *imageResolutionModel MEMBER m_imageResolutionModel CONSTANT)
	Q_PROPERTY(MediaSettingsQualityModel *imageQualityModel MEMBER m_imageQualityModel CONSTANT)

	Q_PROPERTY(AudioEncoderSettings audioEncoderSettings READ audioEncoderSettings WRITE setAudioEncoderSettings NOTIFY audioEncoderSettingsChanged)
	Q_PROPERTY(MediaSettingsAudioCodecModel *audioCodecModel MEMBER m_audioCodecModel CONSTANT)
	Q_PROPERTY(MediaSettingsAudioSampleRateModel *audioSampleRateModel MEMBER m_audioSampleRateModel CONSTANT)
	Q_PROPERTY(MediaSettingsQualityModel *audioQualityModel MEMBER m_audioQualityModel CONSTANT)

	Q_PROPERTY(VideoEncoderSettings videoEncoderSettings READ videoEncoderSettings WRITE setVideoEncoderSettings NOTIFY videoEncoderSettingsChanged)
	Q_PROPERTY(MediaSettingsVideoCodecModel *videoCodecModel MEMBER m_videoCodecModel CONSTANT)
	Q_PROPERTY(MediaSettingsResolutionModel *videoResolutionModel MEMBER m_videoResolutionModel CONSTANT)
	Q_PROPERTY(MediaSettingsVideoFrameRateModel *videoFrameRateModel MEMBER m_videoFrameRateModel CONSTANT)
	Q_PROPERTY(MediaSettingsQualityModel *videoQualityModel MEMBER m_videoQualityModel CONSTANT)

public:
	enum class Type {
		Invalid,
		Image,
		Audio,
		Video
	};
	Q_ENUM(Type)

	enum class AvailabilityStatus {
		Available = QMultimedia::Available,
		ServiceMissing = QMultimedia::ServiceMissing,
		Busy = QMultimedia::Busy,
		ResourceError = QMultimedia::ResourceError
	};
	Q_ENUM(AvailabilityStatus)

	enum class State {
		StoppedState = QMediaRecorder::StoppedState,
		RecordingState = QMediaRecorder::RecordingState,
		PausedState = QMediaRecorder::PausedState
	};
	Q_ENUM(State)

	enum class Status {
		UnavailableStatus = QMediaRecorder::UnavailableStatus,
		UnloadedStatus = QMediaRecorder::UnloadedStatus,
		LoadingStatus = QMediaRecorder::LoadingStatus,
		LoadedStatus = QMediaRecorder::LoadedStatus,
		StartingStatus = QMediaRecorder::StartingStatus,
		RecordingStatus = QMediaRecorder::RecordingStatus,
		PausedStatus = QMediaRecorder::PausedStatus,
		FinalizingStatus = QMediaRecorder::FinalizingStatus
	};
	Q_ENUM(Status)

	enum class Error {
		NoError = QCameraImageCapture::NoError,
		NotReadyError = QCameraImageCapture::NotReadyError,
		ResourceError = QCameraImageCapture::ResourceError,
		OutOfSpaceError = QCameraImageCapture::OutOfSpaceError,
		NotSupportedFeatureError = QCameraImageCapture::NotSupportedFeatureError,
		FormatError = QCameraImageCapture::FormatError
	};
	Q_ENUM(Error)

	explicit MediaRecorder(QObject *parent = nullptr);
	~MediaRecorder() override;

	MediaRecorder::Type type() const;
	void setType(MediaRecorder::Type type);

	QMediaObject *mediaObject() const override;
	QString currentSettingsKey() const;

	MediaRecorder::AvailabilityStatus availabilityStatus() const;
	MediaRecorder::State state() const;
	MediaRecorder::Status status() const;
	bool isAvailable() const;
	bool isReady() const;
	MediaRecorder::Error error() const;
	QString errorString() const;
	QUrl actualLocation() const;
	qint64 duration() const;

	bool isMuted() const;
	void setMuted(bool isMuted);

	qreal volume() const;
	void setVolume(qreal volume);

	MediaSettings mediaSettings() const;
	void setMediaSettings(const MediaSettings  &settings);

	ImageEncoderSettings imageEncoderSettings() const;
	void setImageEncoderSettings(const ImageEncoderSettings &settings);

	AudioEncoderSettings audioEncoderSettings() const;
	void setAudioEncoderSettings(const AudioEncoderSettings &settings);

	VideoEncoderSettings videoEncoderSettings() const;
	void setVideoEncoderSettings(const VideoEncoderSettings &settings);

	// Reset to system default
	Q_INVOKABLE void resetSettingsToDefaults();
	// Reset to user default
	Q_INVOKABLE void resetUserSettings();
	// Save user default
	Q_INVOKABLE void saveUserSettings();

public slots:
	void record();
	void pause();
	void stop();
	void cancel();

signals:
	void typeChanged();
	void availabilityStatusChanged();
	void stateChanged();
	void statusChanged();
	void readyChanged();
	void errorChanged();
	void actualLocationChanged();
	void durationChanged();
	void mutedChanged();
	void volumeChanged();
	void mediaSettingsChanged();
	void imageEncoderSettingsChanged();
	void audioEncoderSettingsChanged();
	void videoEncoderSettingsChanged();

protected:
	bool setMediaObject(QMediaObject *object) override;

	QString settingsKey(MediaRecorder::Type type, const QString &deviceName) const;
	CameraInfo userDefaultCamera() const;
	AudioDeviceInfo userDefaultAudioInput() const;
	void resetSettings(const CameraInfo &camera, const AudioDeviceInfo &audioInput);

	void setupCamera();
	void setupCapturer();
	void setupAudioRecorder();
	void setupVideoRecorder();
	void setupRecorder(MediaRecorder::Type type);

	void deleteActualLocation();

	static QString encoderContainer(const QString &container, const void *userData);
	static QString encoderResolution(const QSize &size, const void *userData);
	static QString encoderQuality(const CommonEncoderSettings::EncodingQuality quality, const void *userData);
	static QString imageEncoderCodec(const QString &codec, const void *userData);
	static QString audioEncoderCodec(const QString &codec, const void *userData);
	static QString audioEncoderSampleRate(const int sampleRate, const void *userData);
	static QString videoEncoderCodec(const QString &codec, const void *userData);
	static QString videoEncoderFrameRate(const qreal frameRate, const void *userData);

private:
	MediaRecorder::Type m_type = MediaRecorder::Type::Invalid;
	bool m_initializing = false;
	QCamera *m_camera = nullptr;
	CameraImageCapture  *m_imageCapturer = nullptr;
	QAudioRecorder *m_audioRecorder = nullptr;
	QMediaRecorder *m_videoRecorder = nullptr;

	MediaSettings m_mediaSettings;
	CameraModel *m_cameraModel = nullptr;
	AudioDeviceModel *m_audioDeviceModel = nullptr;
	MediaSettingsContainerModel *m_containerModel = nullptr;

	ImageEncoderSettings m_imageEncoderSettings;
	MediaSettingsImageCodecModel *m_imageCodecModel = nullptr;
	MediaSettingsResolutionModel *m_imageResolutionModel = nullptr;
	MediaSettingsQualityModel *m_imageQualityModel = nullptr;

	AudioEncoderSettings m_audioEncoderSettings;
	MediaSettingsAudioCodecModel *m_audioCodecModel = nullptr;
	MediaSettingsAudioSampleRateModel *m_audioSampleRateModel = nullptr;
	MediaSettingsQualityModel *m_audioQualityModel = nullptr;

	VideoEncoderSettings m_videoEncoderSettings;
	MediaSettingsVideoCodecModel *m_videoCodecModel = nullptr;
	MediaSettingsResolutionModel *m_videoResolutionModel = nullptr;
	MediaSettingsVideoFrameRateModel *m_videoFrameRateModel = nullptr;
	MediaSettingsQualityModel *m_videoQualityModel = nullptr;
};

DECL_MEDIA_SETTING_MODEL(Container, QString, MediaRecorder::encoderContainer);
DECL_MEDIA_SETTING_MODEL(Quality, CommonEncoderSettings::EncodingQuality, MediaRecorder::encoderQuality);
DECL_MEDIA_SETTING_MODEL(Resolution, QSize, MediaRecorder::encoderResolution);

DECL_MEDIA_SETTING_MODEL(ImageCodec, QString, MediaRecorder::imageEncoderCodec);

DECL_MEDIA_SETTING_MODEL(AudioCodec, QString, MediaRecorder::audioEncoderCodec);
DECL_MEDIA_SETTING_MODEL(AudioSampleRate, int, MediaRecorder::audioEncoderSampleRate);

DECL_MEDIA_SETTING_MODEL(VideoCodec, QString, MediaRecorder::videoEncoderCodec);
DECL_MEDIA_SETTING_MODEL(VideoFrameRate, qreal, MediaRecorder::videoEncoderFrameRate);
