#!/bin/bash -e

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..

ARCH=$(dpkg-architecture -qDEB_HOST_ARCH)
DEB_HOST_MULTIARCH=$(dpkg-architecture -qDEB_HOST_MULTIARCH)

CLICK_TARGET_DIR="$KAIDAN_SOURCES/bin/ubuntu-touch/tmp/"

! [ -d $CLICK_TARGET_DIR ] && mkdir $CLICK_TARGET_DIR


echo "*****************************************"
echo "Downloading Debian packages"
echo "*****************************************"

echo "Kirigami2"
for PKG in qml-module-org-kde-kirigami2 kirigami2-dev libkf5kirigami2-5; do
	VERSION="5.43.0+p16.04+git20180221.0430-0"
	curl --progress-bar https://archive.neon.kde.org/dev/unstable/pool/main/k/kirigami2/${PKG}_${VERSION}_$ARCH.deb -o /tmp/${PKG}_${VERSION}_$ARCH.deb
	dpkg-deb -x /tmp/${PKG}_${VERSION}_$ARCH.deb $CLICK_TARGET_DIR
	rm /tmp/${PKG}_${VERSION}_$ARCH.deb
done

echo "QtQuickControls2"
for PKG in qml-module-qtquick-controls2 libqt5quickcontrols2-5 qtquickcontrols2-5-dev qml-module-qtquick-templates2 qml-module-qt-labs-platform libqt5quicktemplates2-5 libqt5quicktemplates2-5; do
	VERSION="5.9.3-0ubports2"
	curl --progress-bar https://repo.ubports.com/pool/xenial/main/q/qtquickcontrols2-opensource-src/${PKG}_${VERSION}_$ARCH.deb -o /tmp/${PKG}_${VERSION}_$ARCH.deb
	dpkg-deb -x /tmp/${PKG}_${VERSION}_$ARCH.deb $CLICK_TARGET_DIR
	rm /tmp/${PKG}_${VERSION}_$ARCH.deb
done

echo "Gloox"
for PKG in libgloox-dev libgloox17; do
	VERSION="1.0.20-1+16.04+xenial+build1"
	curl --progress-bar http://neon.plasma-mobile.org:8080/pool/main/g/gloox/${PKG}_${VERSION}_$ARCH.deb -o /tmp/${PKG}_${VERSION}_$ARCH.deb
	dpkg-deb -x /tmp/${PKG}_${VERSION}_$ARCH.deb $CLICK_TARGET_DIR
	rm /tmp/${PKG}_${VERSION}_$ARCH.deb
done


echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"

! [ -d $KAIDAN_SOURCES/build ] && mkdir $KAIDAN_SOURCES/build
cd $KAIDAN_SOURCES/build
cmake .. -GNinja -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja -DCMAKE_INSTALL_PREFIX=/usr/ -DI18N=1 -DUBUNTU_TOUCH=1
DESTDIR=$CLICK_TARGET_DIR ninja install

sed -e "s/@ARCH@/$ARCH/g" $KAIDAN_SOURCES/misc/ubuntu-touch/manifest.json.in > $CLICK_TARGET_DIR/manifest.json
cp $KAIDAN_SOURCES/misc/ubuntu-touch/apparmor.json $CLICK_TARGET_DIR
cp $KAIDAN_SOURCES/misc/ubuntu-touch/kaidan.desktop $CLICK_TARGET_DIR
cp $KAIDAN_SOURCES/misc/kaidan.svg $CLICK_TARGET_DIR

# Strip out documentation and includes
rm -r \
	$CLICK_TARGET_DIR/usr/include \
	$CLICK_TARGET_DIR/usr/share/doc \
	$CLICK_TARGET_DIR/usr/lib/$DEB_HOST_MULTIARCH/cmake \
	$CLICK_TARGET_DIR/usr/lib/$DEB_HOST_MULTIARCH/pkgconfig \
	$CLICK_TARGET_DIR/usr/lib/$DEB_HOST_MULTIARCH/qt5/mkspecs

# Move everything to correct location for Ubuntu Touch
mv $CLICK_TARGET_DIR/usr/lib/$DEB_HOST_MULTIARCH/qt5/qml/* $CLICK_TARGET_DIR/usr/lib/$DEB_HOST_MULTIARCH
mv $CLICK_TARGET_DIR/usr/* $CLICK_TARGET_DIR/

echo "*****************************************"
echo "Build script finished, now leaving work to 'click build'"
echo "******************************************"
