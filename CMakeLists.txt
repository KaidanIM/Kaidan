cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 14)
set(QT_MIN_VERSION "5.8.0")

project(kaidan)

# CMake options
option(I18N "Enable i18n support" FALSE)
option(SAILFISH_OS "Build with Sailfish OS GUI, needs 3rdparty/kaidansf submodule" FALSE)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc, uic and rcc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# CMake modules include
include(FeatureSummary)
include(GNUInstallDirs)

#
# Dependecies
#

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Ubuntu Touch click package building specific path
# For internal use by clickable only
if(UBUNTU_TOUCH)
	set(CMAKE_PREFIX_PATH ${CMAKE_PREFIX_PATH} "${CMAKE_SOURCE_DIR}/bin/ubuntu-touch/tmp/usr")
	include_directories("${CMAKE_SOURCE_DIR}/bin/ubuntu-touch/tmp/usr/include")
endif()

# Find packages
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Qml Quick Sql QuickControls2)
find_package(KF5Kirigami2 REQUIRED)
find_package(Gloox REQUIRED)

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
	if(EXISTS "${CMAKE_SOURCE_DIR}/3rdparty/kaidansf/CMakeLists.txt")
		include("${CMAKE_SOURCE_DIR}/3rdparty/kaidansf/CMakeLists.txt")
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
	${GLOOX_LIBRARIES}
)


#
# Global C++ variables
#

if(SAILFISH_OS)
	set(KAIDAN_COMPILE_DEFINITIONS
		SAILFISH_OS=True
		PLATFORM_DISPLAY_NAME="Sailfish OS"
	)
elseif(UBUNTU_TOUCH)
	set(KAIDAN_COMPILE_DEFINITIONS
		UBUNTU_TOUCH=True
	)
else()
	set(KAIDAN_COMPILE_DEFINITIONS
		NOTIFICATIONS_LIBNOTIFY=True
	)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
	DEBUG_SOURCE_PATH="${CMAKE_SOURCE_DIR}"
	${KAIDAN_COMPILE_DEFINITIONS}
)


#
# Install Kaidan
#

if(UNIX AND NOT APPLE AND NOT UBUNTU_TOUCH)
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
elseif(UBUNTU_TOUCH)
	# install kaidan binary
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
		DESTINATION ${CMAKE_INSTALL_FULL_BINDIR}
	)

	# install kaidan media
	install(DIRECTORY "data/images"
		DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/im.kaidan.kaidan"
	)

	# install icon
	install(FILES "misc/kaidan.svg"
		DESTINATION "${CMAKE_INSTALL_PREFIX}"
	)

endif()


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
