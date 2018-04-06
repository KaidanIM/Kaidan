TEMPLATE = app

QT += qml quick quickcontrols2 svg sql
android: QT += androidextras

CONFIG += c++11

SOURCES += \
    src/main.cpp \
    src/Kaidan.cpp \
    src/ClientThread.cpp \
    src/ClientWorker.cpp \
    src/AvatarFileStorage.cpp \
    src/Database.cpp \
    src/RosterModel.cpp \
    src/RosterManager.cpp \
    src/RosterUpdater.cpp \
    src/MessageHandler.cpp \
    src/MessageSessionHandler.cpp \
    src/MessageModel.cpp \
    src/Notifications.cpp \
    src/PresenceCache.cpp \
    src/PresenceHandler.cpp \
    src/ServiceDiscoveryManager.cpp \
    src/VCardManager.cpp \
    src/XmlLogHandler.cpp \
    src/StatusBar.cpp \
    src/HttpUploadHandler.cpp \
    src/QtHttpUploader.cpp \
    src/singleapp/singleapplication.cpp \
    src/singleapp/singleapplication_p.cpp \
    src/gloox-extensions/httpuploadmanager.cpp \
    src/gloox-extensions/httpuploadrequest.cpp \
    src/gloox-extensions/httpuploadslot.cpp \
    src/gloox-extensions/bitsofbinarydata.cpp \
    src/gloox-extensions/bitsofbinarymanager.cpp \
    src/gloox-extensions/bitsofbinarymemorycache.cpp \
    src/gloox-extensions/reference.cpp

HEADERS += \
    src/Database.h \
    src/RosterModel.h \
    src/XmlLogHandler.h  \
    src/RosterManager.h \
    src/AvatarFileStorage.h \
    src/ServiceDiscoveryManager.h \
    src/RosterUpdater.h \
    src/MessageSessionHandler.h \
    src/MessageModel.h \
    src/Notifications.h \
    src/PresenceCache.h \
    src/PresenceHandler.h \
    src/MessageHandler.h \
    src/Kaidan.h \
    src/ClientThread.h \
    src/ClientWorker.h \
    src/VCardManager.h \
    src/Globals.h \
    src/Enums.h \
    src/StatusBar.h \
    src/HttpUploadHandler.h \
    src/QtHttpUploader.h \
    src/singleapp/singleapplication.h \
    src/singleapp/singleapplication_p.h \
    src/gloox-extensions/httpuploadmanager.h \
    src/gloox-extensions/httpuploadrequest.h \
    src/gloox-extensions/httpuploadslot.h \
    src/gloox-extensions/httpuploadhandler.h \
    src/gloox-extensions/httpuploader.h \
    src/gloox-extensions/bitsofbinarydata.h \
    src/gloox-extensions/bitsofbinarymanager.h \
    src/gloox-extensions/bitsofbinaryhandler.h \
    src/gloox-extensions/bitsofbinarycache.h \
    src/gloox-extensions/bitsofbinarymemorycache.h \
    src/gloox-extensions/reference.h


android: INCLUDEPATH += $$PWD/3rdparty/gloox/include
android: LIBS += -L$$PWD/3rdparty/gloox/lib/
android: LIBS += -L$$PWD/3rdparty/openssl/lib/
LIBS += -lgloox -lssl -lcrypto

include(3rdparty/kirigami/kirigami.pri)

RESOURCES += kaidan_qml.qrc i18n/0_i18n.qrc data/images/images.qrc

TRANSLATIONS += \
    i18n/base.ts \
    i18n/de.ts \
    i18n/es.ts \
    i18n/fr.ts \
    i18n/ja.ts \
    i18n/ms.ts \
    i18n/nb.ts \
    i18n/nl.ts \
    i18n/ru.ts \
    i18n/pt.ts

DEFINES += QMAKE_BUILD

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH = $$PWD/src/qml $$PWD/3rdparty/kirigami/src

# Additional import path used to resolve QML modules just for Qt Quick Designer
QML_DESIGNER_IMPORT_PATH = $$PWD/src/qml $$PWD/3rdparty/kirigami/src

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    misc/android/AndroidManifest.xml \
    misc/android/gradle/wrapper/gradle-wrapper.jar \
    misc/android/gradlew \
    misc/android/res/values/libs.xml \
    misc/android/build.gradle \
    misc/android/gradle/wrapper/gradle-wrapper.properties \
    misc/android/gradlew.bat \
    misc/android/AndroidManifest.xml \
    misc/android/gradle/wrapper/gradle-wrapper.jar \
    misc/android/gradlew \
    misc/android/res/drawable/splash.xml \
    misc/android/res/values/colors.xml \
    misc/android/res/values/libs.xml \
    misc/android/res/values/theme.xml \
    misc/android/res/mipmap-hdpi/icon.png \
    misc/android/res/mipmap-hdpi/logo.png \
    misc/android/res/mipmap-ldpi/icon.png \
    misc/android/res/mipmap-ldpi/logo.png \
    misc/android/res/mipmap-mdpi/icon.png \
    misc/android/res/mipmap-mdpi/logo.png \
    misc/android/res/mipmap-xhdpi/icon.png \
    misc/android/res/mipmap-xhdpi/logo.png \
    misc/android/res/mipmap-xxhdpi/icon.png \
    misc/android/res/mipmap-xxhdpi/logo.png \
    misc/android/res/mipmap-xxxhdpi/icon.png \
    misc/android/res/mipmap-xxxhdpi/logo.png \
    misc/android/gradle/wrapper/gradle-wrapper.properties \
    misc/android/build.gradle \
    misc/android/gradlew.bat \
    misc/android/gradle.properties \
    misc/android/local.properties

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/misc/android

contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        $$PWD/3rdparty/openssl/lib/libcrypto.so \
        $$PWD/3rdparty/openssl/lib/libssl.so
}
