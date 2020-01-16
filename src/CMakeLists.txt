# set Kaidans sources (used in main cmake file)
set(KAIDAN_SOURCES
	src/main.cpp
	src/Kaidan.cpp
	src/ClientWorker.cpp
	src/AvatarFileStorage.cpp
	src/Database.cpp
	src/RosterItem.cpp
	src/RosterModel.cpp
	src/RosterFilterProxyModel.cpp
	src/RosterDb.cpp
	src/RosterManager.cpp
	src/RegistrationManager.cpp
	src/Message.cpp
	src/MessageModel.cpp
	src/MessageDb.cpp
	src/MessageHandler.cpp
	src/Notifications.cpp
	src/PresenceCache.cpp
	src/DiscoveryManager.cpp
	src/VCardManager.cpp
	src/VCardModel.cpp
	src/LogHandler.cpp
	src/StatusBar.cpp
	src/UploadManager.cpp
	src/EmojiModel.cpp
	src/TransferCache.cpp
	src/DownloadManager.cpp
	src/QmlUtils.cpp
	src/Utils.cpp
	src/QrCodeDecoder.cpp
	src/QrCodeGenerator.cpp
	src/QrCodeScannerFilter.cpp
	src/QrCodeVideoFrame.cpp
	src/CameraModel.cpp
	src/AudioDeviceModel.cpp
	src/MediaSettings.cpp
	src/CameraImageCapture.cpp
	src/MediaUtils.cpp
	src/MediaRecorder.cpp
	src/CredentialsValidator.cpp

	# needed to trigger moc generation / to be displayed in IDEs
	src/Enums.h
	src/Globals.h

	# kaidan QXmpp extensions (need to be merged into QXmpp upstream)
	src/qxmpp-exts/QXmppHttpUploadIq.cpp
	src/qxmpp-exts/QXmppUploadRequestManager.cpp
	src/qxmpp-exts/QXmppUploadManager.cpp
	src/qxmpp-exts/QXmppColorGenerator.cpp
	src/qxmpp-exts/QXmppUri.cpp

	# hsluv-c required for color generation
	src/hsluv-c/hsluv.c
)

if(NOT ANDROID AND NOT IOS)
	set(KAIDAN_SOURCES ${KAIDAN_SOURCES}
		src/singleapp/singleapplication.cpp
		src/singleapp/singleapplication_p.cpp
	)
endif()
