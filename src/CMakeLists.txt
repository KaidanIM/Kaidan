# set Kaidans sources (used in main cmake file)
set(KAIDAN_SOURCES
	src/main.cpp
	src/Kaidan.cpp
	src/ClientWorker.cpp
	src/AvatarFileStorage.cpp
	src/Database.cpp
	src/RosterModel.cpp
	src/RosterManager.cpp
	src/RegistrationManager.cpp
	src/MessageHandler.cpp
	src/MessageModel.cpp
	src/Notifications.cpp
	src/PresenceCache.cpp
	src/DiscoveryManager.cpp
	src/VCardManager.cpp
	src/LogHandler.cpp
	src/StatusBar.cpp
	src/UploadManager.cpp
	src/EmojiModel.cpp
	src/TransferCache.cpp
	src/DownloadManager.cpp
	src/QmlUtils.cpp

	# needed to trigger moc generation
	src/Enums.h

	# kaidan QXmpp extensions (need to be merged into QXmpp upstream)
	src/qxmpp-exts/QXmppHttpUploadIq.cpp
	src/qxmpp-exts/QXmppUploadRequestManager.cpp
	src/qxmpp-exts/QXmppUploadManager.cpp
	src/qxmpp-exts/QXmppColorGenerator.cpp

        # hsluv-c required for color generation
	src/hsluv-c/hsluv.c
)

if(NOT ANDROID AND NOT IOS)
	set(KAIDAN_SOURCES ${KAIDAN_SOURCES}
		src/singleapp/singleapplication.cpp
		src/singleapp/singleapplication_p.cpp
	)
endif()
