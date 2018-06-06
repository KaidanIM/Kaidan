#!/bin/bash -e
. utils/travis/common.sh

echo "*****************************************"
echo "Building Kaidan"
echo "^^^^^^^^^^^^^^^"
echo_env
echo "*****************************************"

mkdir -p ${TRAVIS_BUILD_DIR}/build
cd ${TRAVIS_BUILD_DIR}/build

if [[ ${PLATFORM} == "ubuntu-touch" ]]; then
	cd ..
	clickable clean build click-build
elif [[ ${BUILD_SYSTEM} == "cmake" ]]; then
	cmake .. \
	      -GNinja \
	      -DCMAKE_BUILD_TYPE=Debug \
	      -DI18N=1 \
	      -DCMAKE_CXX_COMPILER_LAUNCHER=ccache

	cmake --build .
elif [[ ${BUILD_SYSTEM} == "qmake" ]]; then
	# generate binary translations (qm files)
	lrelease ../Kaidan.pro

	qmake ../Kaidan.pro \
	      "QMAKE_CXX=ccache ${CXX}" \
	      "CONFIG+=debug"

	make -j${CPUS_USED}
else
	echo "Unknown platform or build system!"
	exit 1
fi
