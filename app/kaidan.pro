# path to local compiled swift 3 lib
SWIFT3PATH = ../../../swift-3.0
# from swift-config
SWIFTCXX = -DSWIFTEN_STATIC -DBOOST_ALL_NO_LIB -DBOOST_SYSTEM_NO_DEPRECATED -DBOOST_SIGNALS_NO_DEPRECATION_WARNING -DSWIFT_EXPERIMENTAL_FT
SWIFTLIB = -lSwiften -lSwiften_Boost -lrt -lz -lssl -lcrypto -lxml2 -lresolv -lpthread -ldl -lm -lc -lstdc++


TEMPLATE = app
QT += qml quick core

INCLUDEPATH += $${SWIFT3PATH}/3rdParty/Boost/src
INCLUDEPATH += $${SWIFT3PATH}/

QMAKE_CXXFLAGS += $${SWIFTCXX}
LIBS += -L$${SWIFT3PATH}/Swiften -L$${SWIFT3PATH}/3rdParty/Boost $${SWIFTLIB}

DEFINES += BOOST_SIGNALS_NO_DEPRECATION_WARNING

SOURCES += source/main.cpp \
    source/Kaidan.cpp \
    source/RosterContoller.cpp \
    source/RosterItem.cpp

HEADERS += source/Kaidan.h \
    source/EchoPayload.h \
    source/EchoPayloadParserFactory.h \
    source/EchoPayloadSerializer.h \
    source/RosterContoller.h \
    source/RosterItem.h

RESOURCES += resources/qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(resources/deployment.pri)
