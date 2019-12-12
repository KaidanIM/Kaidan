# Kaidan i18n

find_package(Qt5LinguistTools)

file(GLOB FILES_TO_TRANSLATE
	${CMAKE_SOURCE_DIR}/src/*.cpp
	${CMAKE_SOURCE_DIR}/src/qml/*.qml
	${CMAKE_SOURCE_DIR}/src/qml/elements/*.qml
	${CMAKE_SOURCE_DIR}/src/qml/settings/*.qml
)
file(GLOB TS_FILES
	${CMAKE_CURRENT_LIST_DIR}/*.ts
)


#
# Update TS-files
#

set(LUPDATE_ARGS -silent)

foreach(TS_FILE ${TS_FILES})
add_custom_command(
	OUTPUT ${TS_FILE}
	COMMAND ${Qt5_LUPDATE_EXECUTABLE} ${LUPDATE_ARGS} ${FILES_TO_TRANSLATE} -ts ${TS_FILE}
	COMMENT "Updating ${TS_FILE}"
	MAIN_DEPENDENCY ${FILES_TO_TRANSLATE}
	DEPENDS ${FILES_TO_TRANSLATE}
	VERBATIM
)
endforeach()

# extra target for *only* updating ts files
add_custom_target(ts_update)
foreach(TS_FILE ${TS_FILES})
add_custom_command(
	TARGET ts_update
	COMMAND ${Qt5_LUPDATE_EXECUTABLE} ${LUPDATE_ARGS} ${FILES_TO_TRANSLATE} -ts ${TS_FILE}
	COMMENT "Updating ${TS_FILE}"
)
endforeach()


#
# Generate QM-files
#

set(LRELEASE_ARGS -compress -nounfinished -removeidentical -silent)

foreach(TS_FILE ${TS_FILES})
get_filename_component(FILENAME ${TS_FILE} NAME_WE) # only name without extension
set(QM_FILE "${FILENAME}.qm")       # name for the generated qm file
set(QM_FILES ${QM_FILES} "${CMAKE_CURRENT_BINARY_DIR}/${QM_FILE}")

add_custom_command(
	OUTPUT ${QM_FILE}
	COMMAND ${Qt5_LRELEASE_EXECUTABLE} ${LRELEASE_ARGS} ${TS_FILE} -qm ${CMAKE_CURRENT_BINARY_DIR}/${QM_FILE}
	COMMENT "Generating binary translation file ${QM_FILE}"
	DEPENDS ${TS_FILE}
	VERBATIM
)
endforeach()



#
# Add all translations to the binary via. rcc / qrc file
#

set(I18N_QRC_NAME translations)
set(I18N_QRC_INPUT ${CMAKE_CURRENT_LIST_DIR}/${I18N_QRC_NAME}.qrc)
set(I18N_QRC_COPY ${CMAKE_CURRENT_BINARY_DIR}/kaidan_i18n.qrc)
set(I18N_QRC_CPP ${CMAKE_CURRENT_BINARY_DIR}/qrc_kaidan_i18n.cpp)
set_property(SOURCE ${CMAKE_CURRENT_BINARY_DIR}/qrc_kaidan_i18n.cpp PROPERTY SKIP_AUTOGEN ON)

# Copy the QRC file to the output directory, because the files listed in the
# qrc file are relative to that directory.
add_custom_command(
	OUTPUT ${I18N_QRC_COPY}
	COMMAND ${CMAKE_COMMAND} -E copy ${I18N_QRC_INPUT} ${I18N_QRC_COPY}
	COMMENT "Copying i18n resource file to current build directory"
	MAIN_DEPENDENCY ${I18N_QRC_INPUT}
)

# Run the resource compiler (rcc_options should already be set). We can't
# use QT5_ADD_RESOURCES because the qrc file may not exist yet.
add_custom_command(
	OUTPUT ${I18N_QRC_CPP}
	COMMAND ${Qt5Core_RCC_EXECUTABLE} ${RCC_OPTIONS} -name ${I18N_QRC_NAME} -o ${I18N_QRC_CPP} ${I18N_QRC_COPY}
	COMMENT "Generating qrc source qrc_kaidan_i18n.cpp"
	MAIN_DEPENDENCY ${I18N_QRC_COPY}
	DEPENDS ${QM_FILES}
)


# Translatiopns target
add_custom_target(translations ALL DEPENDS ${TS_FILES} ${QM_FILES})
