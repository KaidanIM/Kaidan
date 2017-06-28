set(CURDIR ${CMAKE_CURRENT_LIST_DIR})

# set Kaidans sources (used in main cmake file)
set(KAIDAN_SOURCES
	${CURDIR}/main.cpp
	${CURDIR}/Kaidan.cpp
	${CURDIR}/Database.cpp
	${CURDIR}/RosterModel.cpp
	${CURDIR}/RosterManager.cpp
	${CURDIR}/RosterUpdater.cpp
	${CURDIR}/MessageHandler.cpp
	${CURDIR}/MessageSessionHandler.cpp
	${CURDIR}/MessageModel.cpp
	${CURDIR}/Notifications.cpp
	${CURDIR}/PresenceController.cpp
	${CURDIR}/ServiceDiscoveryManager.cpp
	${CURDIR}/VCard.cpp
	${CURDIR}/VCardController.cpp
)
