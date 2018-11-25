#!/bin/bash -e
. utils/travis/common.sh

echo "*****************************************"
echo "Building Kaidan"
echo "^^^^^^^^^^^^^^^"
echo_env
echo "*****************************************"
echo

mkdir -p ${BUILD_DIR}/build
cd ${BUILD_DIR}/build

if [[ ${PLATFORM} == "ubuntu-touch" ]]; then
	cd ..
	git submodule update --init --remote --checkout
	clickable clean build click-build review
	if [ ! -z $OPENSTORE_API_KEY ]; then
		clickable publish
	fi
	mv bin/ubuntu-touch/*.click .
elif [[ ${BUILD_SYSTEM} == "cmake" ]]; then
	cmake .. \
	      -GNinja \
	      -DCMAKE_BUILD_TYPE=Debug \
	      -DI18N=1 \
	      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

	cmake --build .
else
	echo "Unknown platform or build system!"
	exit 1
fi
