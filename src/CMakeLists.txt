set(CURDIR ${CMAKE_CURRENT_LIST_DIR})

# set Kaidans sources (used in main cmake file)
set(KAIDAN_SOURCES
	${CURDIR}/main.cpp
	${CURDIR}/Kaidan.cpp
	${CURDIR}/ClientThread.cpp
	${CURDIR}/ClientWorker.cpp
	${CURDIR}/AvatarFileStorage.cpp
	${CURDIR}/ChatStateCache.cpp
	${CURDIR}/Database.cpp
	${CURDIR}/RosterModel.cpp
	${CURDIR}/RosterManager.cpp
	${CURDIR}/RosterUpdater.cpp
	${CURDIR}/MessageSessionHandler.cpp
	${CURDIR}/MessageHandler.cpp
	${CURDIR}/MessageModel.cpp
	${CURDIR}/ChatStateHandler.cpp
	${CURDIR}/Notifications.cpp
	${CURDIR}/PresenceHandler.cpp
	${CURDIR}/ServiceDiscoveryManager.cpp
	${CURDIR}/VCardManager.cpp
	${CURDIR}/XmlLogHandler.cpp
	${CURDIR}/StatusBar.cpp

	# needed to trigger moc generation
	${CURDIR}/Enums.h
)
