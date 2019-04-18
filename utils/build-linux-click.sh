#!/bin/bash -e

KAIDAN_SOURCES=$(dirname "$(readlink -f "${0}")")/..
CLICK_TARGET_DIR="$KAIDAN_SOURCES/bin/ubuntu-touch/tmp" # tmp is hard-coded into clickable

mkdir -p $CLICK_TARGET_DIR

DATE=$(date +%Y%m%d.%H%M)
ARCH=$(dpkg-architecture -qDEB_HOST_ARCH)
DEB_HOST_MULTIARCH=$(dpkg-architecture -qDEB_HOST_MULTIARCH)
BUILD_DIR="build/${DEB_ARCH_MULTIARCH}"

build_kaidan() {
	mkdir -p $KAIDAN_SOURCES/$BUILD_DIR
	cd $KAIDAN_SOURCES/$BUILD_DIR
	export PKG_CONFIG_PATH=$PKG_CONFIG_PATH:$CLICK_TARGET_DIR/lib/$DEB_HOST_MULTIARCH/pkgconfig:$CLICK_TARGET_DIR/lib/pkgconfig
	export LDFLAGS="${LDFLAGS} -L${CLICK_TARGET_DIR}/lib/$DEB_HOST_MULTIARCH -L${CLICK_TARGET_DIR}/lib"

	cmake .. \
	      -GNinja \
	      -DCMAKE_MAKE_PROGRAM=/usr/bin/ninja \
	      -DCMAKE_PREFIX_PATH="${CLICK_TARGET_DIR}" \
	      -DCMAKE_INSTALL_PREFIX="${CLICK_TARGET_DIR}" \
	      -DCMAKE_CXX_FLAGS=-isystem\ ${CLICK_TARGET_DIR}/include/qxmpp \
	      -DI18N=1 \
	      -DUBUNTU_TOUCH=1 \
	      -DCLICK_ARCH="${ARCH}" \
	      -DCLICK_DATE="${DATE}" \
	      -DQUICK_COMPILER=OFF \
	      -DCMAKE_BUILD_TYPE=Release

	ninja install
}

cleanup_click_dir() {
	# Strip out documentation and includes
	rm -r \
		$CLICK_TARGET_DIR/include \
		$CLICK_TARGET_DIR/lib/${DEB_HOST_MULTIARCH}/{cmake,pkgconfig}
}



echo "*****************************************"
echo "Building Kaidan"
echo "*****************************************"

build_kaidan

cleanup_click_dir

echo "*****************************************"
echo "Build script finished, now leaving work to 'click build'"
echo "******************************************"
