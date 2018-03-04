#!/bin/bash -e
. utils/travis/common.sh

echo "*****************************************"
echo "Building Kaidan"
echo "Building for platform ${PLATFORM}"
echo "Using build system: ${BUILD_SYSTEM}"
echo "Number of CPUs used: $(nproc)"
echo "*****************************************"

#env_setup

mkdir -p ${TRAVIS_BUILD_DIR}/build
cd ${TRAVIS_BUILD_DIR}/build

if  [ ${PLATFORM} == "linux-desktop" ] && [[ ${BUILD_SYSTEM} == "cmake" ]]; then
	cmake .. \
	      -GNinja \
	      -DCMAKE_BUILD_TYPE=Debug \
	      -DI18N=1 \
	      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

	cmake --build .
elif [ ${PLATFORM} == "linux-desktop" ] && [[ ${BUILD_SYSTEM} == "qmake" ]]; then
	# generate binary translations (qm files)
	lrelease ../Kaidan.pro

	qmake ../Kaidan.pro \
	      "QMAKE_CXX=ccache g++" \
	      "CONFIG+=debug"

	make -j$(nproc)
elif [ ${PLATFORM} == "ubuntu-touch" ]; then
	cd ..;
	clickable clean build click-build
else
	echo "Unknown platform \"${PLATFORM}\""
	exit 1
fi
