cmake_minimum_required(VERSION 3.3)
set(CMAKE_CXX_STANDARD 17)
set(QT_MIN_VERSION "5.14.0")
set(KF5_MIN_VERSION "5.67.0")

if(POLICY CMP0071)
	cmake_policy(SET CMP0071 NEW)
endif()

project(kaidan)

# application information
set(APPLICATION_ID "im.kaidan.kaidan")
set(APPLICATION_NAME "kaidan")
set(APPLICATION_DISPLAY_NAME "Kaidan")
set(APPLICATION_DESCRIPTION "A simple, user-friendly Jabber/XMPP client for every device!")
if(UBUNTU_TOUCH)
	set(APPLICATION_NAME "${APPLICATION_ID}")
endif()

# Version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 7)
set(VERSION_PATCH 0)
set(VERSION_CODE 11)
set(VERSION_EXTRA "" CACHE STRING "Stuff to append to version string")

set(DEVELOPMENT_BUILD FALSE)
if(DEVELOPMENT_BUILD)
	set(VERSION_EXTRA "${VERSION_EXTRA}-dev")
endif()

set(VERSION_STRING "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")
if(VERSION_EXTRA)
	set(VERSION_STRING ${VERSION_STRING}${VERSION_EXTRA})
endif()

# CMake options
option(I18N "Enable i18n support" FALSE)
option(BUILD_TESTS "Build unit tests" FALSE)
option(STATIC_BUILD "Build Kaidan statically")
option(APPIMAGE "Build Kaidan as AppImage (will only work in the appimage script)" FALSE)
option(UBUNTU_TOUCH "Building an Ubuntu Touch click (internal use only!)" FALSE)
option(CLICK_ARCH "Architecture that will be used in the click's manifest")
option(CLICK_DATE "Date used in the version number in the click's manifest")
option(QUICK_COMPILER "Use QtQuick compiler to improve performance" FALSE)
option(USE_KNOTIFICATIONS "Use KNotifications for displaying notifications" TRUE)
option(BUNDLE_ICONS "Bundle breeze icons" FALSE)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc, uic and rcc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(AUTOMOC_MOC_OPTIONS -Muri=${APPLICATION_ID})

#
# Dependecies
#

find_package(ECM 5.40.0 REQUIRED NO_MODULE)

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

# CMake modules include
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(FeatureSummary)
kde_enable_exceptions()

# Find packages
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Concurrent Qml Quick Svg Sql QuickControls2 Xml Multimedia Positioning Location)
find_package(KF5Kirigami2 ${KF5_MIN_VERSION} REQUIRED)
find_package(ZXing REQUIRED)
find_package(QXmpp 1.2.0 REQUIRED)

# Optional QtQuickCompiler
if(QUICK_COMPILER)
	find_package(Qt5QuickCompiler)
	set_package_properties(Qt5QuickCompiler PROPERTIES
		DESCRIPTION "Compile QML at build time"
		TYPE OPTIONAL
	)
endif()

if(USE_KNOTIFICATIONS)
	find_package(KF5Notifications REQUIRED)
	set(__KF5Notifications_LIBRARIES KF5::Notifications)
	add_definitions(-DHAVE_KNOTIFICATIONS)
endif()

# Optional QWidget style integration (not on mobile)
if(NOT UBUNTU_TOUCH AND NOT ANDROID AND NOT IOS)
	find_package(Qt5Widgets)
	find_package(KF5QQC2DeskopStyle)
	set_package_properties(Qt5Widgets PROPERTIES
		DESCRIPTION "Integration with QWidget based desktop styles"
		TYPE OPTIONAL
	)
endif()

# Platform-specific
if(Qt5Widgets_FOUND)
	add_definitions(-DHAVE_QWIDGETS -DQAPPLICATION_CLASS=QApplication)
	set(__Qt5Widgets_LIBRARIES Qt5::Widgets)
else()
	add_definitions(-DQAPPLICATION_CLASS=QGuiApplication)
endif()

if(ANDROID)
	find_package(Qt5 REQUIRED COMPONENTS AndroidExtras)
endif()

if(ANDROID)
	find_package(PkgConfig REQUIRED)
	pkg_search_module(OPENSSL REQUIRED openssl IMPORTED_TARGET)
	message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
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

# Include bundled icons on Ubuntu Touch, Android, Windows, macOS and iOS
if(BUNDLE_ICONS OR UBUNTU_TOUCH OR ANDROID)
	set(KAIDAN_ICONS_QRC kirigami-icons.qrc)
endif()

# Bundled knotifications configuration files on platforms that require it
if(ANDROID)
	set(KAIDAN_NOTIFICATIONS_QRC "misc/notifications.qrc")
endif()

# Bundle images on Android, Windows, macOS and iOS
if(ANDROID OR WIN32 OR APPLE)
	set(KAIDAN_IMAGES_QRC "data/images/images.qrc")
endif()

# Set app icon
include(ECMAddAppIcon)
file(GLOB ICONS_SRCS "${CMAKE_SOURCE_DIR}/misc/app-icons/*kaidan.png")
message(STATUS ${ICONS_SRCS})
ecm_add_app_icon(KAIDAN_ICNS ICONS ${ICONS_SRCS})
message(STATUS ${KAIDAN_ICNS})

if(QUICK_COMPILER)
	qtquick_compiler_add_resources(KAIDAN_QML_QRC src/qml/qml.qrc)
else()
	qt5_add_resources(KAIDAN_QML_QRC src/qml/qml.qrc)
endif()

# misc resources (e.g. qtquickcontrols2.conf)
qt5_add_resources(KAIDAN_MISC_QRC misc/misc.qrc)

add_executable(${PROJECT_NAME}
	WIN32
	${KAIDAN_ICNS}
	${KAIDAN_SOURCES}
	${KAIDAN_QML_QRC}
	${KAIDAN_MISC_QRC}
	${KAIDAN_ICONS_QRC}          # only set if enabled
	${KAIDAN_IMAGES_QRC}         #
	${I18N_QRC_CPP}              #
	${KAIDAN_NOTIFICATIONS_QRC}  #
	"data/data.qrc"
)

target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Sql
	Qt5::Qml
	Qt5::Quick
	Qt5::Svg
	Qt5::Network
	Qt5::Xml
	Qt5::Multimedia
	Qt5::Positioning
	Qt5::Location
	Qt5::QuickControls2
	QXmpp::QXmpp
	${__Qt5Widgets_LIBRARIES}
	${__KF5Notifications_LIBRARIES}
)
if(TARGET ZXing::ZXing)
        target_link_libraries(${PROJECT_NAME} ZXing::ZXing)
elseif(TARGET ZXing::Core)
        target_link_libraries(${PROJECT_NAME} ZXing::Core)
endif()

if(ANDROID OR IOS)
	target_link_libraries(${PROJECT_NAME} Qt5::QuickControls2 KF5::Kirigami2)
endif()

if(ANDROID)
	target_link_libraries(${PROJECT_NAME} PkgConfig::OPENSSL)
endif()

if(ANDROID)
	target_link_libraries(${PROJECT_NAME} Qt5::AndroidExtras)
endif()

if(STATIC_BUILD)
	add_definitions(-DQXMPP_BUILD)

	find_package(Perl REQUIRED)

	set(STATIC_DEPENDENCIES_CMAKE_FILE "${CMAKE_BINARY_DIR}/QtStaticDependencies.cmake")
	if(EXISTS ${STATIC_DEPENDENCIES_CMAKE_FILE})
		file(REMOVE ${STATIC_DEPENDENCIES_CMAKE_FILE})
	endif()

	get_target_property(QT_LIBDIR Qt5::Core LOCATION)
	get_filename_component(QT_LIBDIR ${QT_LIBDIR} DIRECTORY)

	macro(CONVERT_PRL_LIBS_TO_CMAKE _qt_component)
		if(TARGET Qt5::${_qt_component})
			get_target_property(_lib_location Qt5::${_qt_component} LOCATION)
			execute_process(COMMAND ${PERL_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/utils/convert-prl-libs-to-cmake.pl
				--lib ${_lib_location}
				--libdir ${QT_LIBDIR}
				--out ${STATIC_DEPENDENCIES_CMAKE_FILE}
				--component ${_qt_component}
				--compiler ${CMAKE_CXX_COMPILER_ID}
			)
		endif()
	endmacro()

	link_directories(${_qt5_install_prefix}/../)

	foreach(qt_module Gui Quick QuickControls2 Network Qml Svg Sql QSQLiteDriverPlugin
             QJpegPlugin QGifPlugin QSvgPlugin QSvgIconPlugin QICOPlugin QGenericEnginePlugin
	         QLocalClientConnectionFactory QTcpServerConnectionFactory)
		CONVERT_PRL_LIBS_TO_CMAKE(${qt_module})
	endforeach()

	if(IOS)
		foreach(qt_module QIOSIntegrationPlugin QMacHeifPlugin QMacJp2Plugin
			QICNSPlugin QTgaPlugin QTiffPlugin QWbmpPlugin QWebpPlugin)
			CONVERT_PRL_LIBS_TO_CMAKE(${qt_module})
		endforeach()
	endif()

	if(NOT EXISTS ${STATIC_DEPENDENCIES_CMAKE_FILE})
		message(FATAL_ERROR "Unable to find ${STATIC_DEPENDENCIES_CMAKE_FILE}")
	endif()
	include(${STATIC_DEPENDENCIES_CMAKE_FILE})

	set(QT_QML_PATH ${_qt5Quick_install_prefix})

	find_library(KIRIGAMI_PLUGIN kirigamiplugin PATHS ${Kirigami2_INSTALL_PREFIX}/${KDE_INSTALL_QMLDIR}/org/kde/kirigami.2)
	find_library(QUICK_PLUGIN qtquick2plugin PATHS ${QT_QML_PATH}/qml/QtQuick.2)
	find_library(LABS_PLATFORM_PLUGIN qtlabsplatformplugin PATHS ${QT_QML_PATH}/qml/Qt/labs/platform)
	find_library(GRAPHEFFECTS_PLUGIN qtgraphicaleffectsplugin PATHS ${QT_QML_PATH}/qml/QtGraphicalEffects)
	find_library(GRAPHEFFECTS_PRIVATE_PLUGIN qtgraphicaleffectsprivate PATHS ${QT_QML_PATH}/qml/QtGraphicalEffects/private)
	find_library(QQC2_PLUGIN qtquickcontrols2plugin PATHS ${QT_QML_PATH}/qml/QtQuick/Controls.2)
	find_library(QQC2_MATERIAL_PLUGIN qtquickcontrols2materialstyleplugin PATHS ${QT_QML_PATH}/qml/QtQuick/Controls.2/Material)
	find_library(QQC2_UNIVERSAL_PLUGIN qtquickcontrols2universalstyleplugin PATHS ${QT_QML_PATH}/qml/QtQuick/Controls.2/Universal)
	find_library(QLAYOUTS_PLUGIN qquicklayoutsplugin PATHS ${QT_QML_PATH}/qml/QtQuick/Layouts)
	find_library(QWINDOW_PLUGIN windowplugin PATHS ${QT_QML_PATH}/qml/QtQuick/Window.2)
	find_library(QSHAPES_PLUGIN qmlshapesplugin PATHS ${QT_QML_PATH}/qml/QtQuick/Shapes)
	find_library(QUICKSHAPES Qt5QuickShapes PATHS ${QT_LIBDIR})
	find_library(QTEMPLATES_PLUGIN qtquicktemplates2plugin PATHS ${QT_QML_PATH}/qml/QtQuick/Templates.2)
	find_library(QMODELS_PLUGIN modelsplugin PATHS ${QT_QML_PATH}/qml/QtQml/Models.2)
	target_link_libraries(${PROJECT_NAME}
		${plugin_libs}
		Qt5::QSQLiteDriverPlugin
		Qt5::QJpegPlugin
		Qt5::QGifPlugin
		Qt5::QSvgPlugin
		Qt5::QSvgIconPlugin
		Qt5::QICOPlugin
		Qt5::QGenericEnginePlugin
		Qt5::QLocalClientConnectionFactory
		Qt5::QTcpServerConnectionFactory
		${KIRIGAMI_PLUGIN}
		${QUICK_PLUGIN}
		${LABS_PLATFORM_PLUGIN}
		${GRAPHEFFECTS_PLUGIN}
		${GRAPHEFFECTS_PRIVATE_PLUGIN}
		${QQC2_PLUGIN}
		${QQC2_MATERIAL_PLUGIN}
		${QQC2_UNIVERSAL_PLUGIN}
		${QLAYOUTS_PLUGIN}
		${QWINDOW_PLUGIN}
		${QSHAPES_PLUGIN}
		${QUICKSHAPES}
		${QTEMPLATES_PLUGIN}
		${QMODELS_PLUGIN}
		${__Qt5Widgets_LIBRARIES}
	)

	if(IOS)
		target_link_libraries(${PROJECT_NAME}
			Qt5::QIOSIntegrationPlugin
			Qt5::QMacHeifPlugin
			Qt5::QMacJp2Plugin
			Qt5::QICNSPlugin
			Qt5::QTgaPlugin
			Qt5::QTiffPlugin
			Qt5::QWbmpPlugin
			Qt5::QWebpPlugin
		)
	endif()
endif()

# Set a custom plist file for the app bundle
if(APPLE)
	if(IOS)
		set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/misc/ios/Info.plist)
	else()
		set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/misc/macos/Info.plist)
	endif()
endif()

# iOS-specific linker flags
if(IOS)
	set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-Wl,-e,_qt_main_wrapper -u _qt_registerPlatformPlugin")
endif()

#
# Global C++ variables
#

if(UBUNTU_TOUCH)
	set(KAIDAN_COMPILE_DEFINITIONS UBUNTU_TOUCH=True)
elseif(APPIMAGE)
	set(KAIDAN_COMPILE_DEFINITIONS
		APPIMAGE=True
		TARGET_GSTREAMER_PLUGINS="${TARGET_GSTREAMER_PLUGINS}"
	)
endif()

if(STATIC_BUILD)
	set(KAIDAN_COMPILE_DEFINITIONS
		STATIC_BUILD=True
		${KAIDAN_COMPILE_DEFINITIONS}
	)
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE
	DEBUG_SOURCE_PATH="${CMAKE_SOURCE_DIR}"
	VERSION_STRING="${VERSION_STRING}"
	APPLICATION_ID="${APPLICATION_ID}"
	APPLICATION_NAME="${APPLICATION_NAME}"
	APPLICATION_DISPLAY_NAME="${APPLICATION_DISPLAY_NAME}"
	APPLICATION_DESCRIPTION="${APPLICATION_DESCRIPTION}"

	${KAIDAN_COMPILE_DEFINITIONS}
)

#
# Install Kaidan
#

if(ANDROID)
	configure_file(${CMAKE_SOURCE_DIR}/misc/android/AndroidManifest.xml.in
	               ${CMAKE_SOURCE_DIR}/misc/android/AndroidManifest.xml)
elseif(IOS)
	configure_file(${CMAKE_SOURCE_DIR}/misc/ios/Info.plist.in
	               ${CMAKE_SOURCE_DIR}/misc/ios/Info.plist)
endif()

if(UBUNTU_TOUCH AND CLICK_ARCH)
	set(CLICK_VERSION ${VERSION_STRING})
	if(DEVELOPMENT_BUILD)
		set(CLICK_VERSION "${CLICK_VERSION}.${CLICK_DATE}")
	endif()

	# will replace ${CLICK_ARCH} with its value
	configure_file(${CMAKE_SOURCE_DIR}/misc/ubuntu-touch/manifest.json.in
	               ${CMAKE_SOURCE_DIR}/misc/ubuntu-touch/manifest.json)

	# install kaidan binary
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME}
	        DESTINATION ${CMAKE_INSTALL_PREFIX}
	)
	# install kaidan media
	install(DIRECTORY "data/"
	        DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/${APPLICATION_ID}"
		PATTERN "*.qrc" EXCLUDE
	)
	# install icon
	install(FILES "data/images/kaidan.svg" "misc/ubuntu-touch/apparmor.json"
	              "misc/ubuntu-touch/kaidan.desktop" "misc/ubuntu-touch/manifest.json"
	        DESTINATION "${CMAKE_INSTALL_PREFIX}"
	)
elseif(UNIX AND NOT APPLE)
	# install kaidan media
	install(DIRECTORY "data/"
	        DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/${PROJECT_NAME}"
		PATTERN "*.qrc" EXCLUDE
	)

	# install icon (scalable + 128x)
	install(FILES "data/images/kaidan.svg"
	        DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/scalable/apps"
	)
	install(FILES "misc/kaidan-128x128.png"
	        DESTINATION "${CMAKE_INSTALL_FULL_DATADIR}/icons/hicolor/128x128/apps"
	        RENAME "kaidan.png"
	)
	# install desktop file
	install(FILES "misc/kaidan.desktop"
	        DESTINATION "${KDE_INSTALL_APPDIR}"
	)
	# install metainfo
	install(FILES "misc/im.kaidan.kaidan.appdata.xml"
		DESTINATION "${KDE_INSTALL_METAINFODIR}"
		RENAME "${APPLICATION_ID}.appdata.xml"
	)
endif()

# install kaidan binary
install(TARGETS kaidan
        ${KDE_INSTALL_TARGETS_DEFAULT_ARGS}
)

# KNotifications
if(USE_KNOTIFICATIONS)
	install(FILES misc/kaidan.notifyrc DESTINATION ${KNOTIFYRC_INSTALL_DIR})
endif()

#
# Unit tests
#

if(BUILD_TESTS)
	add_subdirectory(tests)
endif()

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
