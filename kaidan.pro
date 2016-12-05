# path to local compiled swift 3 lib
SWIFT3PATH = ../swift-3.0
# from swift-config
SWIFTCXX = -DSWIFTEN_STATIC -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_SIGNALS_NO_DEPRECATION_WARNING -DSWIFT_EXPERIMENTAL_FT
SWIFTLIB = -lSwiften -lSwiften_Boost -lrt -lz -lssl -lcrypto -lxml2 -lresolv -lpthread -ldl -lm -lc -lstdc++


TEMPLATE = app
QT += qml quick core

TARGET = bin/kaidan

INCLUDEPATH += $${SWIFT3PATH}/3rdParty/Boost/src
INCLUDEPATH += $${SWIFT3PATH}/

QMAKE_CXXFLAGS += $${SWIFTCXX}
LIBS += -L$${SWIFT3PATH}/Swiften -L$${SWIFT3PATH}/3rdParty/Boost $${SWIFTLIB}

DEFINES += BOOST_SIGNALS_NO_DEPRECATION_WARNING

SOURCES += src/main.cpp \
	src/Kaidan.cpp \
	src/RosterController.cpp \
	src/RosterItem.cpp

HEADERS += src/Kaidan.h \
	src/EchoPayload.h \
	src/EchoPayloadParserFactory.h \
	src/EchoPayloadSerializer.h \
	src/RosterController.h \
	src/RosterItem.h

RESOURCES += kaidan.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(kaidan.pri)

