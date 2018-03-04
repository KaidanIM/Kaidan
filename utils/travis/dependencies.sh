#!/bin/bash -e
. utils/travis/common.sh

echo "*****************************************"
echo "Fetching dependencies"
echo "Using build system: ${BUILD_SYSTEM}"
echo "*****************************************"

if [ ${PLATFORM} == "linux-desktop" ]; then
	install_linux-desktop_deps
elif [ ${PLATFORM} == "ubuntu-touch" ]; then
	install_ubuntu-touch_deps
else
	echo "Can't build on \"${TRAVIS_OS_NAME}\""
	exit 1
fi
