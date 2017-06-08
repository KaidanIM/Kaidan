cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 14)
set(QT_MIN_VERSION "5.7.0")
set(BOOST_MIN_VERSION "1.34.1")

project(kaidan)

# CMake options
option(I18N "Enable i18n support" FALSE)
option(SAILFISH_OS "Build with Sailfish OS GUI, needs libs/kaidansf submodule" FALSE)

# application name
set(APPLICATION_ID "io.github.kaidanim")
set(APPLICATION_NAME "kaidan")
set(APPLICATION_DISPLAY_NAME "Kaidan")
set(APPLICATION_DESCRIPTION "Cross platform XMPP client")


# Version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 2)
set(VERSION_PATCH 1)
#set(VERSION_EXTRA "dev")
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


#
# Dependecies
#

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find packages
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Qml Quick Sql QuickControls2)
find_package(KF5Kirigami2 REQUIRED)
find_package(Swiften REQUIRED)
find_package(Boost ${BOOST_MIN_VERSION} REQUIRED COMPONENTS signals date_time)
include(GNUInstallDirs)


#
# Load submodules
#

# Main kaidan sources
include("${CMAKE_SOURCE_DIR}/src/CMakeLists.txt")

# I18n support
if(I18N)
	include("${CMAKE_SOURCE_DIR}/i18n/CMakeLists.txt")
endif()

# Sailfish OS GUI
if(SAILFISH_OS)
	if(EXISTS "${CMAKE_SOURCE_DIR}/libs/kaidansf/CMakeLists.txt")
		include("${CMAKE_SOURCE_DIR}/libs/kaidansf/CMakeLists.txt")
	else()
		message(FATAL_ERROR "Could NOT find KaidanSF adaptions for Sailfish OS.")
	endif()
endif()


#
# Sources / Resources
#

# normal kirigami GUI, if no Sailfish OS build
if(NOT SAILFISH_OS)
	set(KAIDAN_QML_QRC kaidan_qml.qrc)
endif()

add_executable(${PROJECT_NAME}
	${KAIDAN_SOURCES}
	${KAIDAN_QML_QRC}
	${I18N_QRC_CPP}     # only set if enabled
	${KAIDANSF_SOURCES} #
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
	Boost::date_time
	${SWIFTEN_LIBRARY}
)


#
# Global C++ variables
#

if(SAILFISH_OS)
	set(KAIDAN_COMPILE_DEFINITIONS
		SAILFISH_OS=True
		PLATFORM_DISPLAY_NAME="Sailfish OS"
	)
else()
	set(KAIDAN_COMPILE_DEFINITIONS
		NOTIFICATIONS_LIBNOTIFY=True
	)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
	APPLICATION_ID="${APPLICATION_ID}"
	APPLICATION_NAME="${APPLICATION_NAME}"
	APPLICATION_DISPLAY_NAME="${APPLICATION_DISPLAY_NAME}"
	APPLICATION_DESCRIPTION="${APPLICATION_DESCRIPTION}"

	VERSION_STRING="${VERSION_STRING}"

	DEBUG_SOURCE_PATH="${CMAKE_SOURCE_DIR}"

	BOOST_SIGNALS_NO_DEPRECATION_WARNING=True

	${KAIDAN_COMPILE_DEFINITIONS}
)


#
# Install Kaidan
#

if(UNIX AND NOT APPLE)
	# install kaidan binary
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	)

	# install kaidan media
	install(DIRECTORY "data/images"
		DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}"
	)

	# install icon (scalable + 128x)
	install(FILES "misc/kaidan.svg"
		DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps"
	)
	install(FILES "misc/kaidan-128x128.png"
		DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/128x128/apps"
		RENAME "kaidan.png"
	)

	# install desktop file
	install(FILES "misc/kaidan.desktop"
		DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/applications"
	)
endif()
