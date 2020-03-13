#!/usr/bin/env bash

# KDE Frameworks Version, either master or a version number
KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..

KF5_VERSION=${KF5_VERSION:-master}
CI_PROJECT_NAME=${CI_PROJECT_NAME:-kaidan}
CI_REPOSITORY_URL=${CI_REPOSITORY_URL:-$(git remote get-url origin)}
CI_PROJECT_DIR=${CI_PROJECT_DIR:-${KAIDAN_SOURCES}}

git submodule update --init

# make sure inkscape is installed
sudo apt update
sudo apt -y install inkscape optipng

# Build dependencies
GIT_EXTRA="--branch ${KF5_VERSION}" /opt/helpers/build-kde-dependencies --withProject kirigami knotifications
/opt/helpers/build-cmake qxmpp https://github.com/qxmpp-project/qxmpp.git -DBUILD_EXAMPLES=OFF -DBUILD_TESTS=OFF
/opt/helpers/build-cmake zxing-cpp https://github.com/nu-book/zxing-cpp.git

# Trick the script to not clone kaidan once again
if ! [ -f "${KAIDAN_SOURCES}/kaidan/" ]; then
    ln -s ${KAIDAN_SOURCES} ${PWD}/src/kaidan
fi

# Build Kaidan
/opt/helpers/build-cmake ${CI_PROJECT_NAME} ${CI_REPOSITORY_URL} -DQTANDROID_EXPORTED_TARGET=kaidan -DANDROID_APK_DIR=${CI_PROJECT_DIR}/src/${CI_PROJECT_NAME}/misc/android -DI18N=1 -DUSE_KNOTIFICATIONS=ON

# pack apk
${CI_PROJECT_DIR}/src/${CI_PROJECT_NAME}/utils/render-logos.sh
/opt/helpers/create-apk ${CI_PROJECT_NAME}

mv ${CI_PROJECT_DIR}/build-arm64/${CI_PROJECT_NAME}/kaidan_build_apk/build/outputs/apk/debug/kaidan_build_apk-debug.apk ${CI_PROJECT_DIR}
