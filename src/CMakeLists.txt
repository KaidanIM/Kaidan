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
# 	${CURDIR}/UploadHandler.cpp
	${CURDIR}/VCardManager.cpp
	${CURDIR}/LogHandler.cpp
	${CURDIR}/StatusBar.cpp
# 	${CURDIR}/QtHttpUploader.cpp

	# SingleApplication
	${CURDIR}/singleapp/singleapplication.cpp
	${CURDIR}/singleapp/singleapplication_p.cpp

	# needed to trigger moc generation
	${CURDIR}/Enums.h

	# kaidan gloox extensions (need to be merged into gloox upstream)
# 	${CURDIR}/gloox-extensions/httpuploadmanager.cpp
# 	${CURDIR}/gloox-extensions/httpuploadrequest.cpp
# 	${CURDIR}/gloox-extensions/httpuploadslot.cpp
# 	${CURDIR}/gloox-extensions/bitsofbinarydata.cpp
# 	${CURDIR}/gloox-extensions/bitsofbinarymanager.cpp
# 	${CURDIR}/gloox-extensions/bitsofbinarymemorycache.cpp
# 	${CURDIR}/gloox-extensions/reference.cpp
# 	${CURDIR}/gloox-extensions/processinghints.cpp
# 	${CURDIR}/gloox-extensions/hash.cpp
# 	${CURDIR}/gloox-extensions/thumb.cpp
# 	${CURDIR}/gloox-extensions/jinglefile.cpp
# 	${CURDIR}/gloox-extensions/sims.cpp
)
