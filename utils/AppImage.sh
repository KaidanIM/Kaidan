#!/bin/bash

echo "*****************************************"
echo "Building Kaidan AppImage. This may take a long time"
echo "*****************************************"

# Download linuxdeployqt
if ! [ -f linuxdeployqt-continuous-x86_64.AppImage ]; then
    wget https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage
    chmod +x linuxdeployqt-continuous-x86_64.AppImage
fi

# configure buildsystem and environment for AppDir installation
if ! [ -d ../build ]; then mkdir ../build; fi
cd ../build
export QT_SELECT=qt5
cmake .. -DCMAKE_INSTALL_PREFIX:PATH=$PWD/../AppDir/usr/

# Build kaidan
make -j$(nproc)
make install
cd ..

# Copy dependencies into AppDir
./utils/linuxdeployqt-continuous-x86_64.AppImage $PWD/AppDir/usr/share/applications/kaidan.desktop -qmldir=/usr/lib/x86_64-linux-gnu/qt5/qml/ -bundle-non-qt-libs

# Create AppImage
./utils/linuxdeployqt-continuous-x86_64.AppImage $PWD/AppDir/usr/share/applications/kaidan.desktop -appimage
