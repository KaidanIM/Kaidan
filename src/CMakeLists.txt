add_compile_definitions(
	QXMPP_DISABLE_DEPRECATED_BEFORE=0x010400
	QT_DISABLE_DEPRECATED_BEFORE=0x050F00
)

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
	src/UserDevicesModel.cpp
	src/DiscoveryManager.cpp
	src/VCardManager.cpp
	src/VCardModel.cpp
	src/LogHandler.cpp
	src/StatusBar.cpp
	src/UploadManager.cpp
	src/EmojiModel.cpp
	src/TransferCache.cpp
	src/DownloadManager.cpp
	src/ServerFeaturesCache.cpp
	src/QmlUtils.cpp
	src/Utils.cpp
	src/VersionManager.cpp
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
	src/CredentialsGenerator.cpp
	src/CredentialsValidator.cpp
	src/BitsOfBinaryImageProvider.cpp
	src/DataFormModel.cpp
	src/RegistrationDataFormFilterModel.cpp
	src/RegistrationDataFormModel.cpp
	src/ServerListModel.cpp
	src/ServerListItem.cpp
	src/AccountManager.cpp
	src/VCardCache.cpp

	# needed to trigger moc generation / to be displayed in IDEs
	src/Enums.h
	src/Globals.h
	src/GuiStyle.h

	# kaidan QXmpp extensions (need to be merged into QXmpp upstream)
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
