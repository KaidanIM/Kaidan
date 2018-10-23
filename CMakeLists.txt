cmake_minimum_required(VERSION 2.8.11)
find_package(ECM 5.40.0 REQUIRED NO_MODULE)
set(CMAKE_CXX_STANDARD 14)
set(QT_MIN_VERSION "5.8.0")

project(kaidan)

set(APPLICATION_ID "im.kaidan.kaidan")

# CMake options
option(I18N "Enable i18n support" FALSE)
option(STATIC_BUILD "Build Kaidan statically")
option(UBUNTU_TOUCH "Building a Ubuntu Touch click (internal use only!)" FALSE)
option(CLICK_ARCH "Architecture that will be used in the click's manifest")
option(CLICK_DATE "Date used in the version number in the click's manifest")

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc, uic and rcc automatically when needed.
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(AUTOMOC_MOC_OPTIONS -Muri=im.kaidan.kaidan)

#
# Dependecies
#

# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

# CMake modules include
include(GNUInstallDirs)
include(KDEInstallDirs)
include(KDECompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)
include(ECMInstallIcons)
include(FeatureSummary)
kde_enable_exceptions()

# Find packages
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Qml Quick Svg Sql QuickControls2)
find_package(KF5Kirigami2 REQUIRED)
find_package(PkgConfig REQUIRED)

pkg_search_module(GLOOX REQUIRED gloox)
include_directories(${GLOOX_INCLUDE_DIRS})

# Optional QWidget style integration (not on mobile)
if(NOT UBUNTU_TOUCH AND NOT ANDROID)
	find_package(Qt5Widgets)
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

if (ANDROID)
	find_package(Qt5 REQUIRED COMPONENTS AndroidExtras)
endif()

if (ANDROID OR WIN32)
	pkg_search_module(OPENSSL REQUIRED openssl)

	include_directories(${OPENSSL_INCLUDE_DIRS})
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
if(UBUNTU_TOUCH OR ANDROID OR WIN32 OR APPLE)
	set(KAIDAN_ICONS_QRC kirigami-icons.qrc)
endif()

# Bundle images on Android, Windows, macOS and iOS
if(ANDROID OR WIN32 OR APPLE)
	set(KAIDAN_IMAGES_QRC "data/images/images.qrc")
endif()

# Set app icon
if(APPLE)
	set(KAIDAN_ICNS "${CMAKE_SOURCE_DIR}/misc/macos/kaidan.icns")
elseif(WIN32)
	include(ECMAddAppIcon)
	file(GLOB WIN_ICONS_SRCS "${CMAKE_SOURCE_DIR}/misc/windows/*kaidan.png")
	message(STATUS ${WIN_ICONS_SRCS})
	ecm_add_app_icon(KAIDAN_ICNS ICONS ${WIN_ICONS_SRCS})
	message(STATUS ${KAIDAN_ICNS})
endif()

add_executable(${PROJECT_NAME}
	MACOSX_BUNDLE
	WIN32
	${KAIDAN_ICNS}
	${KAIDAN_SOURCES}
	${KAIDAN_ICONS_QRC}  # only set if enabled
	${KAIDAN_IMAGES_QRC} #
	${I18N_QRC_CPP}      #
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
	Qt5::Svg
	Qt5::Network
	${__Qt5Widgets_LIBRARIES}
	${GLOOX_LIBRARIES} ${GLOOX_LDFLAGS}
)

if (ANDROID OR WIN32)
	target_link_libraries(${PROJECT_NAME} Qt5::QuickControls2 KF5::Kirigami2 ${OPENSSL_LIBRARIES})
endif()

if (ANDROID)
	target_link_libraries(${PROJECT_NAME} Qt5::AndroidExtras)
endif()

if (WIN32 AND STATIC_BUILD)
	pkg_check_modules(QT5ALL Qt5Gui Qt5Quick Qt5QuickControls2 Qt5Network Qt5Qml Qt5Svg Qt5Sql)
	foreach(plugin ${Qt5Gui_PLUGINS} ${Qt5Network_PLUGINS} ${Qt5Qml_PLUGINS} ${Qt5Svg_PLUGINS} ${Qt5Sql_PLUGINS})
	  get_target_property(_loc ${plugin} LOCATION)
	  message("Plugin ${plugin} is at location ${_loc}")
	  set(plugin_libs ${plugin_libs} ${_loc})
	endforeach()
	pkg_search_module(SQLITE REQUIRED sqlite3)

	set(QT_QML_PATH ${_qt5Quick_install_prefix})
	
	find_library(KIRIGAMI_PLUGIN kirigamiplugin PATHS ${CMAKE_PREFIX_PATH}/${KDE_INSTALL_QMLDIR}/org/kde/kirigami.2)
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
	find_library(QTEMPLATES_PLUGIN qtquicktemplates2plugin PATHS ${QT_QML_PATH}/qml/QtQuick/Templates.2)
	find_library(QMODELS_PLUGIN modelsplugin PATHS ${QT_QML_PATH}/qml/QtQml/Models.2)
	find_library(WIN_SECUR32 secur32)
	find_library(QT_FONT_SUPPORT Qt5FontDatabaseSupport)
	find_library(QT_THEME_SUPPORT Qt5ThemeSupport)
	find_library(QT_EVENT_SUPPORT Qt5EventDispatcherSupport)
	find_library(QT_ACCESSIBILITY_SUPPORT Qt5AccessibilitySupport)
	find_library(QT_UIAUTO_SUPPORT Qt5WindowsUIAutomationSupport)
	target_link_libraries(${PROJECT_NAME}
		${plugin_libs}
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
		${QTEMPLATES_PLUGIN}
		${QMODELS_PLUGIN}
		${SQLITE_LIBRARIES}
		${QT_EVENT_SUPPORT}
		${QT_FONT_SUPPORT}
		${QT_THEME_SUPPORT}
		${QT_ACCESSIBILITY_SUPPORT}
		${QT_UIAUTO_SUPPORT}
		jpeg
		${__Qt5Widgets_LIBRARIES} ${WIN_SECUR32}
		${QT5ALL_LDFLAGS})
endif()

# Set a custom plist file for the app bundle
set_target_properties(${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/misc/macos/Info.plist)

#
# Global C++ variables
#

if(UBUNTU_TOUCH)
	set(KAIDAN_COMPILE_DEFINITIONS
		UBUNTU_TOUCH=True
	)
elseif(NOT ANDROID)
	set(KAIDAN_COMPILE_DEFINITIONS
		NOTIFICATIONS_LIBNOTIFY=True
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
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME}
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
	install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/bin/${PROJECT_NAME}
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
