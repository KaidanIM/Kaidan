#!/bin/bash -e
# NOTE: To use this script, you need to set $QT_IOS to your Qt for iOS installation

if [ -z "$QT_IOS" ]; then
echo "QT_IOS has to be set"
exit 1
fi

# Build type is one of:
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"
PLATFORM="${PLATFORM:-OS64}"

KAIDAN_SOURCES=$(dirname "$(greadlink -f "${0}")")/..
BUILD_FOLDER=$KAIDAN_SOURCES/build-$PLATFORM
KIRIGAMI_BUILD=$KAIDAN_SOURCES/build-deps/kirigami-ios-$PLATFORM
QXMPP_BUILD=$KAIDAN_SOURCES/build-deps/qxmpp-ios-$PLATFORM
ZXING_BUILD=$KAIDAN_SOURCES/build-deps/zxing-ios-$PLATFORM

echo "-- Starting $BUILD_TYPE build of Kaidan --"

echo "*****************************************"
echo "Fetching dependencies if required"
echo "*****************************************"

export PKG_CONFIG_EXECUTABLE=/usr/local/bin/pkg-config

if [ ! -f "$KAIDAN_SOURCES/3rdparty/kirigami/.git" ] || [ ! -f "$KAIDAN_SOURCES/3rdparty/breeze-icons/.git" ]; then
    echo "Cloning Kirigami, Breeze icons, QXmpp and ZXing-cpp"
    git submodule update --init
fi

if [ ! -d "$KAIDAN_SOURCES/3rdparty/ios-cmake/.git" ]; then
    echo "Cloning iOS-cmake"
    git clone https://github.com/leetal/ios-cmake.git 3rdparty/ios-cmake
fi

cdnew() {
    if [ -d "$1" ]; then
        rm -rf "$1"
    fi
    mkdir $1
    cd $1
}

export QT_SELECT=qt5
export DEPLOYMENT_VERSION="10.0"

if [ ! -f "$QXMPP_BUILD/lib/cmake/qxmpp/QXmpp.cmake" ]; then
echo "*****************************************"
echo "Building QXmpp"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/qxmpp/build-$PLATFORM
    cmake .. \
        -DBUILD_SHARED_LIBS=OFF \
        -DCMAKE_PREFIX_PATH=$QT_IOS \
        -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF -DBUILD_SHARED=OFF \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_INSTALL_PREFIX=$QXMPP_BUILD \
        -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake \
        -DPLATFORM=$PLATFORM \
        -DPKG_CONFIG_EXECUTABLE=/usr/local/bin/pkg-config
    make -j$(sysctl -n hw.logicalcpu)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/qxmpp/build
}
fi


if [ ! -f "$KIRIGAMI_BUILD/lib/libKF5Kirigami2.a" ]; then
echo "*****************************************"
echo "Building Kirigami"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/kirigami/build-$PLATFORM

    # workaround for kirigami installation error
    mkdir -p $KAIDAN_SOURCES/3rdparty/kirigami/build-$PLATFORM/src/libkirigami/
    touch $KAIDAN_SOURCES/3rdparty/kirigami/build-$PLATFORM/src/libkirigami/kirigami2_export.h

    cmake .. \
        -DBUILD_SHARED_LIBS=OFF \
        -DECM_DIR=/usr/local/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_IOS \
        -DECM_ADDITIONAL_FIND_ROOT_PATH=$QT_IOS \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$KIRIGAMI_BUILD \
        -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake \
        -DPLATFORM=$PLATFORM

    make -j$(sysctl -n hw.logicalcpu)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/kirigami/build
}
fi

if [ ! -f "$ZXING_BUILD/lib/libZXingCore.a" ]; then
echo "*****************************************"
echo "Building ZXing"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/zxing-cpp/build
    cmake .. \
        -DCMAKE_PREFIX_PATH=$QT_IOS \
        -DECM_ADDITIONAL_FIND_ROOT_PATH=$QT_IOS \
        -DBUILD_SHARED_LIBRARY=OFF \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DCMAKE_INSTALL_PREFIX=$ZXING_BUILD \
        -DCMAKE_TOOLCHAIN_FILE=../../ios-cmake/ios.toolchain.cmake \
        -DPLATFORM=$PLATFORM

    make -j$(nproc)
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

rendersvg()
{
    inkscape -z -e $2 -w $3 -h $3 -d $4 $1
    optipng -quiet $2
}

macoslogo() {
    rendersvg $KAIDAN_SOURCES/misc/kaidan-small-margin.svg "$KAIDAN_SOURCES/misc/macos/kaidan.iconset/icon_$1x$1.png" $1 72
    rendersvg $KAIDAN_SOURCES/misc/kaidan-small-margin.svg "$KAIDAN_SOURCES/misc/macos/kaidan.iconset/icon_$(( $1 * 2 ))x$(( $1 * 2 ))@2x.png" $(( $1 * 2 )) 144
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
    cdnew $KAIDAN_SOURCES/build-$PLATFORM
    cmake .. \
        -GXcode \
        -DPERL_EXECUTABLE=/usr/bin/perl \
        -DSTATIC_BUILD=ON \
        -DECM_DIR=/usr/local/share/ECM/cmake \
        -DCMAKE_PREFIX_PATH=$QT_IOS\;$KIRIGAMI_BUILD\;$QXMPP_BUILD \
        -DKF5Kirigami2_DIR=$KIRIGAMI_BUILD/lib/cmake/KF5Kirigami2 \
        -DAPPLE_SUPPRESS_X11_WARNING=ON \
        -DI18N=1 \
        -DZXing_DIR=$ZXING_BUILD/lib/cmake/ZXing \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DCMAKE_TOOLCHAIN_FILE=../3rdparty/ios-cmake/ios.toolchain.cmake \
        -DPKG_CONFIG_EXECUTABLE=/usr/local/bin/pkg-config \
        -DPLATFORM=$PLATFORM \
        -DARCH="arm64" \
        -DUSE_KNOTIFICATIONS=0 \
        -DQUICK_COMPILER=1

xcodebuild CODE_SIGN_IDENTITY="" CODE_SIGNING_REQUIRED=NO ENABLE_BITCODE=OFF
}
fi
