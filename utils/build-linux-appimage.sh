#!/bin/bash -e

# NOTE: To use this script, you need to set $QT_LINUX to your Qt for Linux installation

# Path to Qt installation
QT_LINUX=${QT_LINUX:-/usr}
echo Using Qt installation from $QT_LINUX

# Build type is one of:
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"

MAKEFLAGS="-j$(nproc)"

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

SOURCE_GSTREAMER_PLUGINS=$(find /opt/kf5/lib -name libgstcoreelements.so -exec dirname {} \;)
SOURCE_GSTREAMER_PLUGINS_SCANNER=$(find /opt/kf5/libexec -name gst-plugin-scanner)
TARGET_GSTREAMER_PLUGINS="find/kf5/lib/$(basename "${SOURCE_GSTREAMER_PLUGINS}")"

echo "*****************************************"
echo "Found GStreamer plugins in: ${SOURCE_GSTREAMER_PLUGINS}"
echo "Found GStreamer plugins scanner in: ${SOURCE_GSTREAMER_PLUGINS_SCANNER}"
echo "Target GStreamer plugins in: ${TARGET_GSTREAMER_PLUGINS}"
echo "*****************************************"

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
        -DQUICK_COMPILER=ON -DAPPIMAGE=ON -DBUNDLE_ICONS=ON \
        -DTARGET_GSTREAMER_PLUGINS="${TARGET_GSTREAMER_PLUGINS}"

    cmake --build .
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

    # We need gstreamer plugins copied first so that their dependencies get deployed as well.
    APPDIR=${KAIDAN_SOURCES}/AppDir
    COPY_GSTREAMER_SCANNER=0

    if [ "$(dirname "${SOURCE_GSTREAMER_PLUGINS_SCANNER}")" != "${TARGET_GSTREAMER_PLUGINS}" ]; then
        COPY_GSTREAMER_SCANNER=1
    fi

    TARGET_GSTREAMER_PLUGINS="${APPDIR}${TARGET_GSTREAMER_PLUGINS}"

    if [ -d "${SOURCE_GSTREAMER_PLUGINS}" ]; then
        echo "Copying ${SOURCE_GSTREAMER_PLUGINS}/libgst* to ${TARGET_GSTREAMER_PLUGINS}..."
        mkdir -p "${TARGET_GSTREAMER_PLUGINS}"
        cp -fr "${SOURCE_GSTREAMER_PLUGINS}/libgst"* "${TARGET_GSTREAMER_PLUGINS}"

        if [ ${COPY_GSTREAMER_SCANNER} -eq 1 ]; then
            echo "Copying ${SOURCE_GSTREAMER_PLUGINS_SCANNER} to ${TARGET_GSTREAMER_PLUGINS}..."
            cp -f "${SOURCE_GSTREAMER_PLUGINS_SCANNER}" "${TARGET_GSTREAMER_PLUGINS}"
        fi
    else
        echo "*****************************************"
        echo "GStreamer plugins not found!!!"
        echo "*****************************************"
        exit 42
    fi

    # linuxdeployqt would only deploy existing binaries in AppDir if they are located in AppDir/usr/lib
    # We do not want to put gstreamer plugins in this location, so they are not deployed.
    # We then do need one -executable=file entry per plugin...
    GSTREAMER_PLUGINS=
    while IFS= read -r gstreamer_plugin; do
        GSTREAMER_PLUGINS="${GSTREAMER_PLUGINS} -executable=\"${gstreamer_plugin}\""
    done < <( find "${TARGET_GSTREAMER_PLUGINS}" -type f )

    eval $KAIDAN_SOURCES/3rdparty/linuxdeployqt/squashfs-root/AppRun \
            $APPDIR/usr/share/applications/kaidan.desktop \
            -qmldir=$KAIDAN_SOURCES/src/qml/ \
            -qmlimport=/opt/kf5/lib/x86_64-linux-gnu/qml \
            -extra-plugins="$(join_by , "${extra_plugins[@]}")" \
            ${GSTREAMER_PLUGINS} \
            -appimage -no-copy-copyright-files \
            $QMAKE_BINARY
}
