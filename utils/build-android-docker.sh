#!/usr/bin/env bash

set -e

function strip_directory_arm() {
	/opt/android-ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/arm-linux-androideabi-strip $(find $1 -name "*armeabi-v7a*".so)
}

function strip_directory_arm64() {
        /opt/android-ndk/toolchains/llvm/prebuilt/linux-x86_64/bin/aarch64-linux-android-strip $(find $1 -name "*arm64*".so)
}

# KDE Frameworks Version, either master or a version number
KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..

KF5_VERSION=${KF5_VERSION:-master}
CI_PROJECT_NAME=${CI_PROJECT_NAME:-kaidan}
CI_REPOSITORY_URL=${CI_REPOSITORY_URL:-$(git remote get-url origin)}
CI_PROJECT_DIR=${CI_PROJECT_DIR:-${KAIDAN_SOURCES}}

git submodule update --init

# make sure inkscape is installed
sudo apt update
sudo apt -y install inkscape optipng advancecomp

# HACK
# We don't want debug symbols, but gradle doesn't strip those anymore seemingly.
# Therefore patch the KDE scripts not to build them into the binary at all.
sudo sed -i 's/-DCMAKE_BUILD_TYPE=debug/-DCMAKE_BUILD_TYPE=Release/g' -i /opt/helpers/build-cmake

# HACK 2
# Strip Qt before building
strip_directory_arm "/opt/Qt/lib/"
strip_directory_arm64 "/opt/Qt/lib/"

# Build dependencies
GIT_EXTRA="--branch ${KF5_VERSION}" /opt/helpers/build-kde-dependencies --withProject kirigami knotifications
/opt/helpers/build-cmake qxmpp https://github.com/qxmpp-project/qxmpp.git -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF
/opt/helpers/build-cmake zxing-cpp https://github.com/nu-book/zxing-cpp.git -DBUILD_BLACKBOX_TESTS=OFF -DBUILD_EXAMPLES=OFF

# Trick the script to not clone kaidan once again
if ! [ -f "${KAIDAN_SOURCES}/kaidan/" ]; then
	ln -s ${KAIDAN_SOURCES} ${PWD}/src/kaidan
fi

# Build Kaidan
/opt/helpers/build-cmake ${CI_PROJECT_NAME} ${CI_REPOSITORY_URL} -DQTANDROID_EXPORTED_TARGET=kaidan -DANDROID_APK_DIR=${CI_PROJECT_DIR}/src/${CI_PROJECT_NAME}/misc/android -DI18N=1 -DUSE_KNOTIFICATIONS=ON

# HACK 3
# Strip build directory before deploying
strip_directory_arm ./build-arm
strip_directory_arm64 ./build-arm64

# pack apk
${CI_PROJECT_DIR}/src/${CI_PROJECT_NAME}/utils/render-logos.sh
/opt/helpers/create-apk ${CI_PROJECT_NAME}

mv ${CI_PROJECT_DIR}/build-arm64/${CI_PROJECT_NAME}/kaidan_build_apk/build/outputs/apk/debug/kaidan_build_apk-debug.apk ${CI_PROJECT_DIR}/kaidan_arm64.apk
mv ${CI_PROJECT_DIR}/build-arm/${CI_PROJECT_NAME}/kaidan_build_apk/build/outputs/apk/debug/kaidan_build_apk-debug.apk ${CI_PROJECT_DIR}/kaidan_arm.apk
