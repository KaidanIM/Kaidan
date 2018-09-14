cmake_minimum_required(VERSION 2.8.11)
set(CMAKE_CXX_STANDARD 14)
set(QT_MIN_VERSION "5.8.0")

project(kaidan)

set(APPLICATION_ID "im.kaidan.kaidan")

# CMake options
option(I18N "Enable i18n support" FALSE)
option(UBUNTU_TOUCH "Building a Ubuntu Touch click (internal use only!)" FALSE)
option(CLICK_ARCH "Architecture that will be used in the click's manifest")
option(CLICK_DATE "Date used in the version number in the click's manifest")

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

# Find packages
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Qml Quick Sql QuickControls2)
find_package(KF5Kirigami2 REQUIRED)
find_package(Gloox REQUIRED)

# optional QWidget style integration (not on ubuntu touch)
if(NOT UBUNTU_TOUCH)
	find_package(Qt5Widgets)
	set_package_properties(Qt5Widgets PROPERTIES
		DESCRIPTION "Integration with QWidget based desktop styles"
		TYPE OPTIONAL
	)
endif()

if(Qt5Widgets_FOUND)
	add_definitions(-DHAVE_QWIDGETS -DQAPPLICATION_CLASS=QApplication)
	set(__Qt5Widgets_LIBRARIES Qt5::Widgets)
else()
	add_definitions(-DQAPPLICATION_CLASS=QGuiApplication)
endif()


#
# Load submodules
#

# Main kaidan sources
include("${CMAKE_SOURCE_DIR}/src/CMakeLists.txt")

# I18n support
if(I18N)
	include("${CMAKE_SOURCE_DIR}/i18n/CMakeLists.txt")
endif()


#
# Sources / Resources
#

# Include bundled icons on Ubuntu Touch,
# because its icon theme is missing some required icons
if(UBUNTU_TOUCH)
	set(KAIDAN_ICONS_QRC kirigami-icons.qrc)
endif()

add_executable(${PROJECT_NAME}
	${KAIDAN_SOURCES}
	${KAIDAN_ICONS_QRC} # only set if enabled
	${I18N_QRC_CPP}     #
	"kaidan_qml.qrc"
)


#
# Linker Flags
#

target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Sql
	Qt5::Qml
	Qt5::Quick
	${__Qt5Widgets_LIBRARIES}
	${GLOOX_LIBRARIES}
)


#
# Global C++ variables
#

if(UBUNTU_TOUCH)
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

if(UBUNTU_TOUCH AND CLICK_ARCH)
	# will replace ${CLICK_ARCH} with its value
	configure_file(${CMAKE_SOURCE_DIR}/misc/ubuntu-touch/manifest.json.in
	               ${CMAKE_SOURCE_DIR}/misc/ubuntu-touch/manifest.json)

	# install kaidan binary
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}
	        DESTINATION ${CMAKE_INSTALL_PREFIX}
	)
	# install kaidan media
	install(DIRECTORY "data/images"
	        DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/${APPLICATION_ID}"
	)
	# install icon
	install(FILES "misc/kaidan.svg" "misc/ubuntu-touch/apparmor.json"
	              "misc/ubuntu-touch/kaidan.desktop" "misc/ubuntu-touch/manifest.json"
	        DESTINATION "${CMAKE_INSTALL_PREFIX}"
	)
elseif(UNIX AND NOT APPLE)
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


feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
