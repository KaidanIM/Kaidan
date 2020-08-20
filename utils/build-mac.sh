#!/bin/bash

# NOTE: To use this script, you need to set $QT_MACOS to your Qt for MacOS installation

if [ -z "$QT_MACOS" ]; then
    echo "QT_MACOS has to be set"
    exit 1
fi

# Build type is one of: 
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"

KAIDAN_SOURCES=$(dirname "$(greadlink -f "${0}")")/..
KIRIGAMI_BUILD=/tmp/kirigami-mac-build
QXMPP_BUILD=/tmp/qxmpp-mac-build
ZXING_BUILD=/tmp/zxing-mac-build

echo "-- Starting $BUILD_TYPE build of Kaidan --"

echo "*****************************************"
echo "Fetching dependencies if required"
echo "*****************************************"

mkdir -p $KAIDAN_SOURCES/3rdparty

if [ ! -f "$KAIDAN_SOURCES/3rdparty/kirigami/.git" ]; then
    echo "Cloning Kirigami"
    git clone https://invent.kde.org/frameworks/kirigami $KAIDAN_SOURCES/3rdparty/kirigami
fi

if [ ! -e "$KAIDAN_SOURCES/3rdparty/breeze-icons/.git" ]; then
    echo "Cloning Breeze icons"
    git clone https://invent.kde.org/frameworks/breeze-icons $KAIDAN_SOURCES/3rdparty/breeze-icons
fi

if [ ! -e "$KAIDAN_SOURCES/3rdparty/qxmpp/.git" ]; then
    echo "Cloning QXmpp"
    git clone https://github.com/qxmpp-project/qxmpp.git $KAIDAN_SOURCES/3rdparty/qxmpp
fi

if [ ! -e "$KAIDAN_SOURCES/3rdparty/zxing-cpp/" ]; then
    echo "Cloning ZXing"
    git clone https://github.com/nu-book/zxing-cpp.git 3rdparty/zxing-cpp
fi


cdnew() {
    if [ -d "$1" ]; then
        rm -rf "$1"
    fi
    mkdir $1
    cd $1
}

export QT_SELECT=qt5

if [ ! -f "$QXMPP_BUILD/lib/pkgconfig/qxmpp.pc" ]; then
echo "*****************************************"
echo "Building QXmpp"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/qxmpp/build
    cmake .. \
	-G"Unix Makefiles" \
        -DCMAKE_PREFIX_PATH=$QT_MACOS \
        -DBUILD_EXAMPLES=OFF \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$QXMPP_BUILD

    make -j$(sysctl -n hw.logicalcpu)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/qxmpp/build
}
fi

if [ ! -f "$KIRIGAMI_BUILD/lib/libKF5Kirigami2.dylib" ]; then
echo "*****************************************"
echo "Building Kirigami"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/kirigami/build
    cmake .. \
        -DECM_DIR=/usr/local/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_MACOS \
        -DECM_ADDITIONAL_FIND_ROOT_PATH=$QT_MACOS \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$KIRIGAMI_BUILD
    
    make -j$(sysctl -n hw.logicalcpu)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/kirigami/build
}
fi

if [ ! -f "$ZXING_BUILD/lib/libZXingCore.dylib" ]; then
echo "*****************************************"
echo "Building ZXing"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/zxing-cpp/build
    cmake .. \
        -DCMAKE_PREFIX_PATH=$QT_MACOS \
        -DBUILD_SHARED_LIBRARY=ON \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$ZXING_BUILD

    make -j$(sysctl -n hw.logicalcpu)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/zxing-cpp/build
}
fi


if [ ! -f "$KAIDAN_SOURCES/misc/macos/kaidan.icns" ]; then
echo "*****************************************"
echo "Rendering logos"
echo "*****************************************"
if [ ! $(command -v inkscape)  ] || [ ! $(command -v optipng) ]; then
echo "Icons can'be generated"
exit 1
fi

rendersvg() {
    inkscape -z -e $2 -w $3 -h $3 -d $4 $1
    optipng -quiet $2
}

macoslogo() {
    rendersvg $KAIDAN_SOURCES/data/images/kaidan.svg "$KAIDAN_SOURCES/misc/macos/kaidan.iconset/icon_$1x$1.png" $1 72
    rendersvg $KAIDAN_SOURCES/data/images/kaidan.svg "$KAIDAN_SOURCES/misc/macos/kaidan.iconset/icon_$(( $1 * 2 ))x$(( $1 * 2 ))@2x.png" $(( $1 * 2 )) 144
}

mkdir -p $KAIDAN_SOURCES/misc/macos/kaidan.iconset

macoslogo 16
macoslogo 32
macoslogo 128
macoslogo 256
macoslogo 512

iconutil --convert icns "$KAIDAN_SOURCES/misc/macos/kaidan.iconset"
fi

export PKG_CONFIG_PATH=$QXMPP_BUILD/lib/pkgconfig

if [ ! -f "$KAIDAN_SOURCES/build/bin/kaidan" ]; then
echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/build

    cmake .. \
        -DECM_DIR=/usr/local/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_MACOS\;$KIRIGAMI_BUILD\;$QXMPP_BUILD \
        -DKF5Kirigami2_DIR=$KIRIGAMI_BUILD/lib/cmake/KF5Kirigami2 -DI18N=1 \
        -DZXing_DIR=$ZXING_BUILD/lib/cmake/ZXing \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DQUICK_COMPILER=1 \
        -DUSE_KNOTIFICATIONS=0

    make -j$(sysctl -n hw.logicalcpu)
}
fi

echo "*****************************************"
echo "Macdeployqt"
echo "*****************************************"
{
    cd $KAIDAN_SOURCES/build
    export LD_LIBRARY_PATH=$QT_MACOS/lib/:$KIRIGAMI_BUILD/lib:$ZXING_BUILD/lib:$LD_LIBRARY_PATH
    export PATH=$QT_MACOS/bin/:$PATH
    
    # FIXME: Use `macdeployqt -qmlimport` when QTBUG-70977 is fixed
    if [ ! -d "$QT_MACOS/qml/org/kde/kirigami.2" ]; then
        mkdir -p $QT_MACOS/qml/org/kde
        ln -s $KIRIGAMI_BUILD/lib/qml/org/kde/kirigami.2 $QT_MACOS/qml/org/kde/kirigami.2
    fi

    macdeployqt bin/kaidan.app -qmldir=../src/qml/ -libpath=$KIRIGAMI_BUILD/lib/ -dmg -appstore-compliant
}
