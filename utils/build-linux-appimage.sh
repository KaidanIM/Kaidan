#!/bin/bash -e

# NOTE: To use this script, you need to set $QT_LINUX to your Qt for Linux installation

# Path to Qt installation
QT_LINUX=${QT_LINUX:-/usr}
echo Using Qt installation from $QT_LINUX

# Build type is one of:
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..

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

join_by() {
    local IFS="$1"
    shift
    echo "$*"
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

if [ ! -f "$KAIDAN_SOURCES/build/bin/kaidan" ]; then
echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/build

    cmake .. \
        -DECM_DIR=/usr/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_LINUX\; \
        -DI18N=1 \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=/usr \
        -DQUICK_COMPILER=ON -DAPPIMAGE=ON -DBUNDLE_ICONS=ON

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
    export LD_LIBRARY_PATH=$QT_LINUX/lib/:$LD_LIBRARY_PATH
    export PATH=$QT_LINUX/bin/:$PATH
    # set qmake binary when using portable Qt; linuxdeployqt will find it on its
    # own on global installs
    if [ -f $QT_LINUX/bin/qmake ]; then
        QMAKE_BINARY="-qmake=$QT_LINUX/bin/qmake"
    fi

    export VERSION="continuous"
    extra_plugins=(
        # Image formats
        imageformats/libqsvg.so
        imageformats/libqjpeg.so
        imageformats/libqgif.so
        imageformats/libqwebp.so
        # Icon formats
        iconengines/libqsvgicon.so
        # QtMultimedia
        audio/libqtaudio_alsa.so
        audio/libqtmedia_pulse.so
        playlistformats/libqtmultimedia_m3u.so
        mediaservice/libgstaudiodecoder.so
        mediaservice/libgstcamerabin.so
        mediaservice/libgstmediacapture.so
        mediaservice/libgstmediaplayer.so
        # QtLocation
        geoservices/libqtgeoservices_esri.so
        geoservices/libqtgeoservices_itemsoverlay.so
        geoservices/libqtgeoservices_mapbox.so
        geoservices/libqtgeoservices_mapboxgl.so
        geoservices/libqtgeoservices_nokia.so
        geoservices/libqtgeoservices_osm.so
        # QtPositioning
        position/libqtposition_geoclue.so
        position/libqtposition_positionpoll.so
        position/libqtposition_serialnmea.so
    )

    $KAIDAN_SOURCES/3rdparty/linuxdeployqt/squashfs-root/AppRun \
        $KAIDAN_SOURCES/AppDir/usr/share/applications/kaidan.desktop \
        -qmldir=$KAIDAN_SOURCES/src/qml/ \
        -qmlimport=/opt/kf5/lib/x86_64-linux-gnu/qml \
        -extra-plugins="$(join_by , "${extra_plugins[@]}")" \
        -appimage -no-copy-copyright-files \
        $QMAKE_BINARY
}
