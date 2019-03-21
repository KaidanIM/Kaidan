#!/bin/bash -e

# NOTE: To use this script, you need to set $QT_LINUX to your Qt for Linux installation

# Path to Qt installation
QT_LINUX=${QT_LINUX:-/usr}
echo Using Qt installation from $QT_LINUX

# Build type is one of:
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..
KIRIGAMI_BUILD=/tmp/kirigami-linux-build
QXMPP_BUILD=${QXMPP_BUILD:-/tmp/qxmpp-linux-build}

echo "-- Starting $BUILD_TYPE build of Kaidan --"

echo "*****************************************"
echo "Fetching dependencies if required"
echo "*****************************************"

if [ ! -f "$KAIDAN_SOURCES/3rdparty/kirigami/.git" ] || [ ! -f "$KAIDAN_SOURCES/3rdparty/breeze-icons/.git" ]; then
    echo "Cloning Kirigami and Breeze icons"
    git submodule update --init
fi

if [ ! -e "$KAIDAN_SOURCES/3rdparty/qxmpp/.git" ]; then
    echo "Cloning QXmpp"
    git clone https://github.com/qxmpp-project/qxmpp.git 3rdparty/qxmpp
fi

cdnew() {
    if [ -d "$1" ]; then
        rm -rf "$1"
    fi
    mkdir $1
    cd $1
}

if [ ! -f "$KAIDAN_SOURCES/3rdparty/linuxdeployqt/squashfs-root/AppRun" ]; then
    echo "Downloading linuxdeployqt"
    wget --continue -P $KAIDAN_SOURCES/3rdparty/ https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
    chmod +x $KAIDAN_SOURCES/3rdparty/linuxdeployqt-continuous-x86_64.AppImage

    echo "Extracting linuxdeployqt"
    cdnew $KAIDAN_SOURCES/3rdparty/linuxdeployqt
    $KAIDAN_SOURCES/3rdparty/linuxdeployqt-continuous-x86_64.AppImage --appimage-extract
    cd $KAIDAN_SOURCES
fi

export QT_SELECT=qt5

if ! $(find $QXMPP_BUILD -name libqxmpp.so >/dev/null); then
echo "*****************************************"
echo "Building QXmpp"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/qxmpp/build
    cmake .. \
        -DCMAKE_PREFIX_PATH=$QT_LINUX \
        -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$QXMPP_BUILD \
        -DCMAKE_INSTALL_LIBDIR=$QXMPP_BUILD/lib

    make -j$(nproc)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/qxmpp/build
}
fi

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

export PKG_CONFIG_PATH=$QXMPP_BUILD/lib/pkgconfig

if [ ! -f "$KAIDAN_SOURCES/build/bin/kaidan" ]; then
echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/build

    cmake .. \
        -DECM_DIR=/usr/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_LINUX\;$KIRIGAMI_BUILD\;$QXMPP_BUILD \
        -DI18N=1 \
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
    # set qmake binary when using portable Qt; linuxdeployqt will find it on its
    # own on global installs
    if [ -f $QT_LINUX/bin/qmake ]; then
        QMAKE_BINARY="-qmake=$QT_LINUX/bin/qmake"
    fi

    export VERSION="continuous"

    $KAIDAN_SOURCES/3rdparty/linuxdeployqt/squashfs-root/AppRun \
        $KAIDAN_SOURCES/AppDir/usr/share/applications/kaidan.desktop \
        -qmldir=$KAIDAN_SOURCES/src/qml/ \
        -qmlimport=$KIRIGAMI_BUILD/lib/qml/ \
        -extra-plugins="imageformats/libqsvg.so,imageformats/libqjpeg.so,iconengines/libqsvgicon.so" \
        -appimage -no-copy-copyright-files \
        $QMAKE_BINARY
}
