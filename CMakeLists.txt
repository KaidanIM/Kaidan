cmake_minimum_required(VERSION 2.8.11)
set(QT_MIN_VERSION "5.5.0")

project(kaidan)

# Version
set(VERSION_MAJOR 0)
set(VERSION_MINOR 0)
set(VERSION_PATCH 1)
set(VERSION_EXTRA "" CACHE STRING "Stuff to append to version string")

# Version string
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


# CMake module path
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

# Find packages
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Core Qml Quick Sql)
find_package(KF5Kirigami REQUIRED)
find_package(Swiften REQUIRED)
find_package(Boost REQUIRED)


# Sources
add_executable(${PROJECT_NAME}
	kaidan.qrc
	src/main.cpp
	src/Kaidan.cpp
	src/RosterController.cpp
	src/RosterItem.cpp
	src/persistence/Database.cpp
	src/persistence/MessageController.cpp
	src/persistence/Persistence.cpp
)

# Additional include dirs
include_directories("${CMAKE_SOURCE_DIR}/src/persistence")

# Link libraries
target_link_libraries(${PROJECT_NAME}
	Qt5::Core
	Qt5::Qml
	Qt5::Quick
	Qt5::Sql
	${SWIFTEN_LIBRARY}
	${Boost_LIBRARY}
)
