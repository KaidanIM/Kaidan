# Translations

# Do not clean up (delete) files in this directory, when using make clean
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)

file(GLOB FILES_TO_TRANSLATE
	${CMAKE_SOURCE_DIR}/src/*.cpp
	${CMAKE_SOURCE_DIR}/src/qml/*.qml
)
file(GLOB TRANSLATION_FILES ${CMAKE_CURRENT_LIST_DIR}/*.ts)

# Update ts-files
qt5_create_translation(TRANSLATION_MESSAGES ${FILES_TO_TRANSLATE} ${TRANSLATION_FILES} OPTIONS -silent)

# Generate qm-files from ts-files
qt5_add_translation(TRANSLATION_QM ${TRANSLATION_FILES})

add_custom_target(translations ALL DEPENDS ${TRANSLATION_MESSAGES})

if(APPLE AND UNIX) ## OSX
	install(FILES ${TRANSLATION_QM} DESTINATION MultiMC.app/Contents/Resources/translations)
else()
	install(FILES ${TRANSLATION_QM} DESTINATION translations)
endif()
