#!/bin/bash -e

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..
CLICK_TARGET_DIR="$KAIDAN_SOURCES/bin/ubuntu-touch/tmp" # tmp is hard-coded into clickable

mkdir -p $CLICK_TARGET_DIR

ARCH=$(dpkg-architecture -qDEB_HOST_ARCH)
DEB_HOST_MULTIARCH=$(dpkg-architecture -qDEB_HOST_MULTIARCH)

install_deb() {
	BASE_URL="${1}"; PKG="${2}"; VERSION="${3}"
	DEB_NAME="${PKG}_${VERSION}_${ARCH}.deb"

	# download deb using curl with a nice progress bar
	curl -L --progress-bar ${BASE_URL}/${DEB_NAME} -o "/tmp/${DEB_NAME}"
	# install to click
	dpkg-deb -x "/tmp/${DEB_NAME}" ${CLICK_TARGET_DIR}
	# clean up
	rm "/tmp/${DEB_NAME}"
}

install_dependencies() {
	KIRIGAMI_VERSION="5.44.0-2"
	QQC2_VERSION="5.9.3-0ubports3"
	GLOOX_VERSION="1.0.20-1+16.04+xenial+build1"

	echo "I: Installing Kirigami 2"
	for PKG in qml-module-org-kde-kirigami2 kirigami2-dev libkf5kirigami2-5; do
		install_deb http://snapshot.debian.org/archive/debian/20180430T215634Z/pool/main/k/kirigami2 ${PKG} ${KIRIGAMI_VERSION}
	done

	echo "I: Installing QtQuick Controls 2"
	for PKG in qml-module-qtquick-controls2 libqt5quickcontrols2-5 qtquickcontrols2-5-dev qml-module-qtquick-templates2 qml-module-qt-labs-platform libqt5quicktemplates2-5 libqt5quicktemplates2-5; do
		install_deb https://repo.ubports.com/pool/xenial/main/q/qtquickcontrols2-opensource-src ${PKG} ${QQC2_VERSION}
	done

	echo "I: Installing gloox"
	for PKG in libgloox-dev libgloox17; do
		install_deb http://neon.plasma-mobile.org:8080/pool/main/g/gloox ${PKG} ${GLOOX_VERSION}
	done

	echo "I: Installing QML modules"
	mv $CLICK_TARGET_DIR/usr/lib/$DEB_HOST_MULTIARCH/qt5/qml/* $CLICK_TARGET_DIR/usr/lib/$DEB_HOST_MULTIARCH
	echo "I: Installing libraries"
	mv $CLICK_TARGET_DIR/usr/* $CLICK_TARGET_DIR/
}

build_kaidan() {
	mkdir -p $KAIDAN_SOURCES/build
	cd $KAIDAN_SOURCES/build
	cmake .. \
	      -GNinja \
	      -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja \
	      -DCMAKE_PREFIX_PATH="${CLICK_TARGET_DIR}" \
	      -DCMAKE_INSTALL_PREFIX="${CLICK_TARGET_DIR}" \
	      -DGLOOX_INCLUDE_DIR="${CLICK_TARGET_DIR}/include" \
	      -DI18N=1 \
	      -DUBUNTU_TOUCH=1 \
	      -DCLICK_ARCH="${ARCH}"

	ninja install
}

cleanup_click_dir() {
	# Strip out documentation and includes
	rm -r \
		$CLICK_TARGET_DIR/usr \
		$CLICK_TARGET_DIR/include \
		$CLICK_TARGET_DIR/share/doc \
		$CLICK_TARGET_DIR/share/locale \
		$CLICK_TARGET_DIR/lib/$DEB_HOST_MULTIARCH/cmake \
		$CLICK_TARGET_DIR/lib/$DEB_HOST_MULTIARCH/pkgconfig \
		$CLICK_TARGET_DIR/lib/$DEB_HOST_MULTIARCH/qt5/mkspecs
}

echo "*****************************************"
echo "Downloading Debian packages"
echo "*****************************************"

install_dependencies


echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"

build_kaidan

cleanup_click_dir

echo "*****************************************"
echo "Build script finished, now leaving work to 'click build'"
echo "******************************************"
