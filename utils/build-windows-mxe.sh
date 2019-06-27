#!/bin/bash

# NOTE: To use this script, you need to set $MXE_ROOT to your the root of your MXE toolchain

if [ -z "$MXE_ROOT" ]; then
    echo "MXE_ROOT has to be set"
    exit 1
fi

# Build type is one of:
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"
# MXE target is one of:
# i686-w64-mingw32.static, i686-w64-mingw32.shared,
# x86_64-w64-mingw32.static, x86_64-w64-mingw32.shared
MXE_TARGET="${MXE_TARGET:-i686-w64-mingw32.static}"

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..

echo "-- Starting $BUILD_TYPE build of Kaidan --"

echo "*****************************************"
echo "Fetching dependencies if required"
echo "*****************************************"

if [ ! -f "$KAIDAN_SOURCES/3rdparty/kirigami/.git" ] || [ ! -f "$KAIDAN_SOURCES/3rdparty/breeze-icons/.git" ]; then
    echo "Cloning Kirigami and Breeze icons"
    git submodule update --init
fi

if [ ! -d "$KAIDAN_SOURCES/3rdparty/qxmpp/.git" ]; then
    echo "Cloning QXmpp"
    git clone https://github.com/qxmpp-project/qxmpp.git 3rdparty/qxmpp
fi

if [ ! -d $MXE_ROOT ]; then
    if [ ! -z "$INIT_TOOLCHAIN" ]; then
        echo "INIT_TOOLCHAIN is set, cloning the MXE toolchain"
        git clone https://github.com/mxe/mxe.git $MXE_ROOT
    else
        echo "MXE toolchain does not exist and INIT_TOOLCHAIN is not set"
        exit 1
    fi
fi

if [ ! -d $MXE_ROOT ]; then
    if [ ! -z "$INIT_TOOLCHAIN" ]; then
        echo "INIT_TOOLCHAIN is set, cloning the MXE toolchain"
        git clone https://github.com/mxe/mxe.git
    else
        echo "MXE toolchain does not exist and INIT_TOOLCHAIN is not set"
        exit 1
    fi
fi

if [ ! -d $MXE_ROOT/usr/$MXE_TARGET ]; then
    cd $MXE_ROOT
    make MXE_TARGETS="$MXE_TARGET" qtbase qtdeclarative qtquickcontrols2 qtgraphicaleffects qttools --jobs=$(nproc)
fi

export PATH=$MXE_ROOT/usr/bin:$PATH

cdnew() {
    if [ -d "$1" ]; then
        rm -rf "$1"
    fi
    mkdir $1
    cd $1
}

if [ ! -d "$MXE_ROOT/usr/$MXE_TARGET/lib/cmake/KF5Kirigami2" ]; then
echo "*****************************************"
echo "Building Kirigami"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/kirigami/build
    $MXE_TARGET-cmake .. \
        -DECM_DIR=/usr/share/ECM/cmake \
        -DSTATIC_LIBRARY=1 \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE

    make -j$(nproc)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/kirigami/build
}
fi

if [ ! -f "$MXE_ROOT/usr/$MXE_TARGET/lib/pkgconfig/qxmpp.pc" ]; then
echo "*****************************************"
echo "Building QXmpp"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/qxmpp/build
    $MXE_TARGET-cmake .. \
        -DBUILD_TESTS=OFF \
        -DBUILD_EXAMPLES=OFF \
        -DBUILD_SHARED=OFF \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE

    make -j$(nproc)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/qxmpp/build
}
fi

if [ ! -d "$KAIDAN_SOURCES/misc/windows" ]; then
    echo "*****************************************"
    echo "Rendering logos"
    echo "*****************************************"
    rendersvg() {
        inkscape -z -e $2 -w $3 -h $3 $1
        optipng -quiet $2
    }

    winlogo() {
        rendersvg $KAIDAN_SOURCES/misc/kaidan-small-margin.svg "$KAIDAN_SOURCES/misc/windows/$1-kaidan.png" $1
        optipng -o9 "$KAIDAN_SOURCES/misc/windows/$1-kaidan.png"
    }

    mkdir -p $KAIDAN_SOURCES/misc/windows

    winlogo 16
    winlogo 32
    winlogo 48
    winlogo 256
fi

if [ ! -f "$KAIDAN_SOURCES/build/bin/kaidan" ]; then
echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/build

    $MXE_TARGET-cmake .. \
        -DECM_DIR=/usr/share/ECM/cmake -DI18N=1 \
        -DSTATIC_BUILD=1 \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DQUICK_COMPILER=1 \
        -DUSE_KNOTIFICATIONS=OFF

    make -j$(nproc)
}
fi


if [ ! -z "$UPX" ]; then
echo "*****************************************"
echo "Compressing executable"
echo "*****************************************"
{
    upx -9 $KAIDAN_SOURCES/build/bin/kaidan.exe
}
fi
