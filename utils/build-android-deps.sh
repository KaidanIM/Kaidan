#!/bin/bash

# NOTE: To use this script, you need to set ANDROID_NDK_ROOT to your NDK root folder.

KAIDAN_SOURCES=$PWD

echo "*****************************************"
echo "Fetching Gloox and OpenSSL"
echo "*****************************************" 

echo "Cloning Gloox from SVN"
svn co svn://svn.camaya.net/gloox/branches/1.0 /tmp/gloox
echo "Downloading OpenSSL release tarball"
wget https://www.openssl.org/source/openssl-1.0.2n.tar.gz -O /tmp/openssl.tar.gz --continue
echo "Extracting OpenSSL tarball"
mkdir /tmp/openssl
tar -xzvf /tmp/openssl.tar.gz -C /tmp/openssl --strip-components=1
echo "Fetching OpenSSL's Setenv-android.sh"
wget https://wiki.openssl.org/images/7/70/Setenv-android.sh -O /tmp/openssl/Setenv-android.sh --continue
dos2unix /tmp/openssl/Setenv-android.sh
sed -i 's/arm-linux-androideabi-4.8/arm-linux-androideabi-4.9/g' /tmp/openssl/Setenv-android.sh
sed -i 's/android-18/android-21/g' /tmp/openssl/Setenv-android.sh
chmod +x /tmp/openssl/Setenv-android.sh

echo "*****************************************"
echo "Preparing Android toolchain"
echo "*****************************************"

echo "ARM, API 21"
$ANDROID_NDK_ROOT/build/tools/make_standalone_toolchain.py --arch arm --api 21 --install-dir /tmp/android-arm-toolchain

echo "*****************************************"
echo "Building OpenSSL"
echo "*****************************************"

{
    cd /tmp/openssl/
    source ./Setenv-android.sh
    ./config shared --openssldir=$KAIDAN_SOURCES/3rdparty/openssl/
    make all -j$(nproc)
    make install -j$(nproc)
}

echo "*****************************************"
echo "Building Gloox"
echo "*****************************************"

{
    # Add the standalone toolchain to the search path.
    export PATH=/tmp/android-arm-toolchain/bin:$PATH

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
    
    cd /tmp/gloox
    ./autogen.sh
    ./configure --host=arm --with-openssl=$KAIDAN_SOURCES/3rdparty/openssl/ --prefix=$KAIDAN_SOURCES/3rdparty/gloox/
    make install -j$(nproc)
}

echo "Installing"
# TODO

echo "*****************************************"
echo "Cleaning up"
echo "*****************************************"
rm -rf /tmp/gloox
rm -rf /tmp/openssl
rm -rf /tmp/android-arm-toolchain
rm /tmp/openssl.tar.gz
