#!/bin/bash

# NOTE: To use this script, you need to set ANDROID_NDK_ROOT, ANDROID_SDK_ROOT
# and QT_ANDROID to your Qt for Android installation

if [ -z "$ANDROID_NDK_ROOT" ]; then
    echo "ANDROID_NDK_ROOT has to be set"
    exit 1
fi

if [ -z "$ANDROID_SDK_ROOT" ]; then
    echo "ANDROID_SDK_ROOT has to be set"
    exit 1
fi

if [ -z "$QT_ANDROID" ]; then
    echo "QT_ANDROID has to be set"
    exit 1
fi

# Build type is one of: 
# Debug, Release, RelWithDebInfo and MinSizeRel
BUILD_TYPE="${BUILD_TYPE:-Debug}"
# Android SDK Tools version available on the system
ANDROID_SDK_BUILD_TOOLS_REVISION=${ANDROID_SDK_BUILD_TOOLS_REVISION:-25.0.3}
# Build API version
ANDROID_API_VERSION=21

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..
GLOOX_PATH=/tmp/gloox
OPENSSL_PATH=/tmp/openssl
OPENSSL_SETENV=$OPENSSL_PATH/Setenv-android.sh
CUSTOM_ANDROID_TOOLCHAIN=/tmp/android-arm-toolchain

echo "-- Starting $BUILD_TYPE build of Kaidan --"

echo "*****************************************"
echo "Fetching dependencies if required"
echo "*****************************************"

if [ ! -d "$GLOOX_PATH" ]; then
    echo "Cloning Gloox from SVN"
    svn co svn://svn.camaya.net/gloox/branches/1.0 $GLOOX_PATH
fi

if [ ! -d "$OPENSSL_PATH" ]; then
    echo "Cloning OpenSSL into $OPENSSL_PATH"
    git clone --depth=1 git://git.openssl.org/openssl.git $OPENSSL_PATH
fi

if [ ! -f "$OPENSSL_SETENV" ]; then
    echo "Fetching OpenSSL's Setenv-android.sh"
    wget https://wiki.openssl.org/images/7/70/Setenv-android.sh -O $OPENSSL_SETENV --continue
    dos2unix $OPENSSL_SETENV
    sed -i 's/arm-linux-androideabi-4.8/arm-linux-androideabi-4.9/g' $OPENSSL_SETENV
    sed -i 's/android-18/android-$ANDROID_API_VERSION/g' $OPENSSL_SETENV
    chmod +x $OPENSSL_SETENV
fi

if [ ! -f "$KAIDAN_SOURCES/3rdparty/kirigami/.git" ] || [ ! -f "$KAIDAN_SOURCES/3rdparty/breeze-icons/.git" ]; then
    echo "Cloning Kirigami and Breeze icons"
    git submodule update --init
fi

if [ ! -d "$CUSTOM_ANDROID_TOOLCHAIN" ]; then
    echo "*****************************************"
    echo "Preparing Android toolchain"
    echo "*****************************************"

    echo "ARM, API $ANDROID_API_VERSION"
    $ANDROID_NDK_ROOT/build/tools/make_standalone_toolchain.py --arch arm --api $ANDROID_API_VERSION --install-dir $CUSTOM_ANDROID_TOOLCHAIN
fi

if [ ! -f "$CUSTOM_ANDROID_TOOLCHAIN/lib/libssl.so" ]; then
echo "*****************************************"
echo "Building OpenSSL"
echo "*****************************************"

{
    cd $OPENSSL_PATH
    source ./Setenv-android.sh
    export ANDROID_NDK=$ANDROID_NDK_ROOT
    ./Configure shared android-armeabi --prefix=$CUSTOM_ANDROID_TOOLCHAIN
    make build_libs -j$(nproc) SHLIB_VERSION_NUMBER= SHLIB_EXT=.so
    make install_sw SHLIB_VERSION_NUMBER= SHLIB_EXT=.so
}
fi

if [ ! -f "$CUSTOM_ANDROID_TOOLCHAIN/lib/libgloox.a" ]; then
echo "*****************************************"
echo "Building Gloox"
echo "*****************************************"

{
    # Add the standalone toolchain to the search path.
    export PATH=$CUSTOM_ANDROID_TOOLCHAIN/bin:$PATH

    # Tell configure what tools to use.
    export target_host=arm-linux-androideabi
    export AR=$target_host-ar
    export AS=$target_host-clang
    export CC=$target_host-clang
    export CXX=$target_host-clang++
    export LD=$target_host-ld
    export STRIP=$target_host-strip

    # Tell configure what flags Android requires.
    export CFLAGS="-fPIE -fPIC"
    export LDFLAGS="-pie"

    cd $GLOOX_PATH
    ./autogen.sh
    ./configure --host=arm --with-openssl=$CUSTOM_ANDROID_TOOLCHAIN --prefix=$CUSTOM_ANDROID_TOOLCHAIN --with-tests=no --with-examples=no
    make -j$(nproc)
    make install
}
fi

cdnew() {
    if [ -d "$1" ]; then
        rm -rf "$1"
    fi
    mkdir $1
    cd $1
}

if [ ! -f "$CUSTOM_ANDROID_TOOLCHAIN/lib/libKF5Kirigami2.so" ]; then
echo "*****************************************"
echo "Building Kirigami"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/3rdparty/kirigami/build
    cmake .. \
        -DCMAKE_TOOLCHAIN_FILE=/usr/share/ECM/toolchain/Android.cmake \
        -DECM_ADDITIONAL_FIND_ROOT_PATH=$QT_ANDROID  \
        -DCMAKE_PREFIX_PATH=$QT_ANDROID/ \
        -DANDROID_NDK=$ANDROID_NDK_ROOT \
        -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
        -DANDROID_SDK_BUILD_TOOLS_REVISION=$ANDROID_SDK_BUILD_TOOLS_REVISION \
        -DCMAKE_INSTALL_PREFIX=$CUSTOM_ANDROID_TOOLCHAIN \
        -DCMAKE_BUILD_TYPE=$BUILD_TYPE
    make -j$(nproc)
    make install
    rm -rf $KAIDAN_SOURCES/3rdparty/kirigami/build
}
fi

if [ ! -d "$KAIDAN_SOURCES/misc/android/res/mipmap-ldpi" ]; then
    echo "*****************************************"
    echo "Rendering logos"
    echo "*****************************************"
    rendersvg() {
        inkscape -z -e $2 -w $3 -h $3 $1
        optipng -quiet $2
    }
    
    androidlogo() {
        mkdir $KAIDAN_SOURCES/misc/android/res/mipmap-$1
        rendersvg $KAIDAN_SOURCES/misc/kaidan-small-margin.svg "$KAIDAN_SOURCES/misc/android/res/mipmap-$1/icon.png" $2
        rendersvg $KAIDAN_SOURCES/misc/kaidan-small-margin.svg "$KAIDAN_SOURCES/misc/android/res/mipmap-$1/logo.png" $(( $2 * 4 ))
    }
    
    androidlogo ldpi 36
    androidlogo mdpi 48
    androidlogo hdpi 72
    androidlogo xhdpi 96
    androidlogo xxhdpi 144
    androidlogo xxxhdpi 192
fi

echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"
{
    cdnew $KAIDAN_SOURCES/build
    cmake .. \
        -DQTANDROID_EXPORTED_TARGET=kaidan -DECM_DIR=/usr/share/ECM/cmake \
        -DCMAKE_TOOLCHAIN_FILE=/usr/share/ECM/toolchain/Android.cmake \
        -DECM_ADDITIONAL_FIND_ROOT_PATH="$QT_ANDROID;$CUSTOM_ANDROID_TOOLCHAIN" \
        -DCMAKE_PREFIX_PATH=$QT_ANDROID \
        -DANDROID_NDK=$ANDROID_NDK_ROOT -DCMAKE_ANDROID_NDK=$ANDROID_NDK_ROOT \
        -DANDROID_SDK_ROOT=$ANDROID_SDK_ROOT \
        -DANDROID_SDK_BUILD_TOOLS_REVISION=$ANDROID_SDK_BUILD_TOOLS_REVISION \
        -DCMAKE_INSTALL_PREFIX=$CUSTOM_ANDROID_TOOLCHAIN \
        -DANDROID_APK_DIR=../misc/android -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
        -DKF5Kirigami2_DIR=$CUSTOM_ANDROID_TOOLCHAIN/lib/cmake/KF5Kirigami2 -DI18N=1
    make create-apk-kaidan -j$(nproc)
}

if [ ! -z "$INSTALL" ]; then
    echo "*****************************************"
    echo "Installing to device"
    echo "*****************************************"
    
    cd $KAIDAN_SOURCES/build
    make install-apk-kaidan
fi
