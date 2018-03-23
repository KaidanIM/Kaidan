set(CURDIR ${CMAKE_CURRENT_LIST_DIR})

# set Kaidans sources (used in main cmake file)
set(KAIDAN_SOURCES
	${CURDIR}/main.cpp
	${CURDIR}/Kaidan.cpp
	${CURDIR}/ClientThread.cpp
	${CURDIR}/ClientWorker.cpp
	${CURDIR}/AvatarFileStorage.cpp
	${CURDIR}/Database.cpp
	${CURDIR}/RosterModel.cpp
	${CURDIR}/RosterManager.cpp
	${CURDIR}/RosterUpdater.cpp
	${CURDIR}/MessageHandler.cpp
	${CURDIR}/MessageSessionHandler.cpp
	${CURDIR}/MessageModel.cpp
	${CURDIR}/Notifications.cpp
	${CURDIR}/PresenceHandler.cpp
	${CURDIR}/ServiceDiscoveryManager.cpp
	${CURDIR}/HttpUploadHandler.cpp
	${CURDIR}/VCardManager.cpp
	${CURDIR}/XmlLogHandler.cpp
	${CURDIR}/StatusBar.cpp
	${CURDIR}/QtHttpUploader.cpp

	# needed to trigger moc generation
	${CURDIR}/Enums.h

	# kaidan gloox extensions (need to be merged into gloox upstream)
	${CURDIR}/gloox-extensions/httpuploadmanager.cpp
	${CURDIR}/gloox-extensions/httpuploadrequest.cpp
	${CURDIR}/gloox-extensions/httpuploadslot.cpp
)
