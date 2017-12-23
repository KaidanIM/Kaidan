#!/bin/bash

echo "*****************************************"
echo "Building Kaidan binary"
echo "Using build system: ${BUILD_SYSTEM}"
echo "Number of CPUs used: $(nproc)"
echo "*****************************************"

mkdir ../build; cd ../build/

if [ $BUILD_SYSTEM == "cmake" ]; then
	cmake .. -GNinja -DI18N=1
	cmake --build .
elif [ $BUILD_SYSTEM == "qmake" ]; then
	lrelease ../Kaidan.pro
	qmake ..
	make -j$(nproc)
fi
