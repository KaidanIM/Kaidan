cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 14)
set(QT_MIN_VERSION "5.7.0")
set(BOOST_MIN_VERSION "1.34.1")

project(kaidan)

# CMake options
option(I18N "Enable i18n support" FALSE)

# application name
set(APPLICATION_ID "harbour.kaidan")
set(APPLICATION_NAME "kaidan")
set(APPLICATION_DISPLAY_NAME "Kaidan")
set(APPLICATION_DESCRIPTION "Cross platform XMPP client")

set(ORGANIZAITON_NAME "KaidanIM")
set(ORGANIZATION_DOMAIN "kaidanim.github.io")

# Version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(VERSION_EXTRA "" CACHE STRING "Stuff to append to version string")

set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
if(VERSION_EXTRA)
	set(VERSION_STRING ${VERSION_STRING}-${VERSION_EXTRA})
endif()


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc, uic and rcc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)


# Set output path for executables
set(EXECUTABLE_OUTPUT_PATH "bin")


#
# Dependecies
#

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find packages
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Qml Quick Sql)
find_package(KF5Kirigami REQUIRED)
find_package(Swiften REQUIRED)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS signals)
include(GNUInstallDirs)

#
# Translation
#

if(I18N)
	include("${CMAKE_SOURCE_DIR}/i18n/CMakeLists.txt")
endif()

#
# Sources / Resources
#

add_executable(${PROJECT_NAME}
	kaidan.qrc
	${I18N_QRC_CPP} # set in i18n/CMakeLists.txt, if enabled
	src/main.cpp
	src/Kaidan.cpp
	src/RosterController.cpp
	src/RosterModel.cpp
	src/MessageController.cpp
	src/MessageModel.cpp
)

#
# Linker Flags
#

target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Sql
	Qt5::Qml
	Qt5::Quick
	Boost::signals
	${SWIFTEN_LIBRARY}
)

#
# Global C++ variables
#

target_compile_definitions(${PROJECT_NAME} PRIVATE
	APPLICATION_ID="${APPLICATION_ID}"
	APPLICATION_NAME="${APPLICATION_NAME}"
	APPLICATION_DISPLAY_NAME="${APPLICATION_DISPLAY_NAME}"
	APPLICATION_DESCRIPTION="${APPLICATION_DESCRIPTION}"
	ORGANIZAITON_NAME="${ORGANIZAITON_NAME}"
	ORGANIZATION_DOMAIN="${ORGANIZATION_DOMAIN}"

	VERSION_STRING="${VERSION_STRING}"

	BOOST_SIGNALS_NO_DEPRECATION_WARNING=True
)


#
# Install Kaidan binary
#

if(UNIX AND NOT APPLE)
	# install kaidan binary
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_OUTPUT_PATH}/${PROJECT_NAME}
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	)

	# install icon (scalable + 128x)
	install(FILES "misc/kaidan.svg"
		DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps"
	)
	install(FILES "misc/kaidan-128x128.png"
		DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/128x128/apps"
		RENAME "kaidan.png"
	)
endif()
