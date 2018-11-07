#!/bin/bash

# NOTE: To use this script, you need to set $QT_LINUX to your Qt for Linux installation

if [ -z "$QT_LINUX" ]; then
    echo "QT_LINUX has to be set"
    exit 1
fi

# Build type is one of: 
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..
KIRIGAMI_BUILD=/tmp/kirigami-linux-build

echo "-- Starting $BUILD_TYPE build of Kaidan --"

echo "*****************************************"
echo "Fetching dependencies if required"
echo "*****************************************"

if [ ! -f "$KAIDAN_SOURCES/3rdparty/kirigami/.git" ] || [ ! -f "$KAIDAN_SOURCES/3rdparty/breeze-icons/.git" ]; then
    echo "Cloning Kirigami and Breeze icons"
    git submodule update --init
fi

if [ ! -f "$KAIDAN_SOURCES/3rdparty/linuxdeployqt-continuous-x86_64.AppImage" ]; then
    echo "Downloading linuxdeployqt"
    wget -P $KAIDAN_SOURCES/3rdparty/ https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
    chmod +x $KAIDAN_SOURCES/3rdparty/linuxdeployqt-continuous-x86_64.AppImage
fi

cdnew() {
    if [ -d "$1" ]; then
        rm -rf "$1"
    fi
    mkdir $1
    cd $1
}

export QT_SELECT=qt5

if [ ! -f "$KIRIGAMI_BUILD/lib/libKF5Kirigami2.so" ]; then
echo "*****************************************"
echo "Building Kirigami"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/kirigami/build
    cmake .. \
        -DECM_DIR=/usr/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_LINUX \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$KIRIGAMI_BUILD \
        -DCMAKE_INSTALL_LIBDIR=$KIRIGAMI_BUILD/lib

    make -j$(nproc)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/kirigami/build
}
fi

if [ ! -f "$KAIDAN_SOURCES/build/bin/kaidan" ]; then
echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/build

    cmake .. \
        -DECM_DIR=/usr/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_LINUX \
        -DKF5Kirigami2_DIR=$KIRIGAMI_BUILD/lib/cmake/KF5Kirigami2 -DI18N=1 \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=/usr
    
    make -j$(nproc)
}
fi

if [ ! -f "$KAIDAN_SOURCES/AppDir/usr/bin/kaidan" ]; then
echo "*****************************************"
echo "Installing Kaidan"
echo "*****************************************"
{
    cd $KAIDAN_SOURCES/build
    DESTDIR=$KAIDAN_SOURCES/AppDir cmake --build . --target install
}
fi

echo "*****************************************"
echo "Packing into AppImage"
echo "*****************************************"
{
    cd $KAIDAN_SOURCES
    export LD_LIBRARY_PATH=$QT_LINUX/lib/:$KIRIGAMI_BUILD/lib:$LD_LIBRARY_PATH
    export PATH=$QT_LINUX/bin/:$PATH
    
    $KAIDAN_SOURCES/3rdparty/linuxdeployqt-continuous-x86_64.AppImage $KAIDAN_SOURCES/AppDir/usr/share/applications/kaidan.desktop -qmake=$QT_LINUX/bin/qmake -qmldir=$KAIDAN_SOURCES/src/qml/ -qmlimport=$KIRIGAMI_BUILD/lib/qml/ -appimage -no-copy-copyright-files
}
