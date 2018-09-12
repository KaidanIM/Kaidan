#!/bin/bash -e

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..
CLICK_TARGET_DIR="$KAIDAN_SOURCES/bin/ubuntu-touch/tmp" # tmp is hard-coded into clickable

mkdir -p $CLICK_TARGET_DIR

DATE=$(date +%Y%m%d)
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
	GLOOX_VERSION="1.0.20-1+16.04+xenial+build1"

	echo "I: Installing gloox"
	for PKG in libgloox-dev libgloox17; do
		install_deb http://neon.plasma-mobile.org:8080/pool/main/g/gloox ${PKG} ${GLOOX_VERSION}
	done

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
	      -DCLICK_ARCH="${ARCH}" \
	      -DCLICK_DATE="${DATE}" \
	      -DCMAKE_BUILD_TYPE=Release

	ninja install
}

cleanup_click_dir() {
	# Strip out documentation and includes
	rm -r \
		$CLICK_TARGET_DIR/usr \
		$CLICK_TARGET_DIR/include \
		$CLICK_TARGET_DIR/share/doc \
		$CLICK_TARGET_DIR/lib/$DEB_HOST_MULTIARCH/pkgconfig
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
