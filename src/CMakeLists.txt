set(CURDIR ${CMAKE_CURRENT_LIST_DIR})

# set Kaidans sources (used in main cmake file)
set(KAIDAN_SOURCES
	${CURDIR}/main.cpp
	${CURDIR}/Kaidan.cpp
	${CURDIR}/ClientWorker.cpp
	${CURDIR}/AvatarFileStorage.cpp
	${CURDIR}/Database.cpp
	${CURDIR}/RosterModel.cpp
	${CURDIR}/RosterManager.cpp
	${CURDIR}/MessageHandler.cpp
	${CURDIR}/MessageModel.cpp
	${CURDIR}/Notifications.cpp
	${CURDIR}/PresenceCache.cpp
	${CURDIR}/DiscoveryManager.cpp
	${CURDIR}/VCardManager.cpp
	${CURDIR}/LogHandler.cpp
	${CURDIR}/StatusBar.cpp
	${CURDIR}/UploadManager.cpp
	${CURDIR}/EmojiModel.cpp

	# needed to trigger moc generation
	${CURDIR}/Enums.h

	# kaidan QXmpp extensions (need to be merged into QXmpp upstream)
	${CURDIR}/qxmpp-exts/QXmppHttpUploadIq.cpp
	${CURDIR}/qxmpp-exts/QXmppUploadRequestManager.cpp
	${CURDIR}/qxmpp-exts/QXmppUploadManager.cpp
)

if (NOT ANDROID AND NOT IOS)
	set (KAIDAN_SOURCES ${KAIDAN_SOURCES}
		${CURDIR}/singleapp/singleapplication.cpp
		${CURDIR}/singleapp/singleapplication_p.cpp
	)
endif()
