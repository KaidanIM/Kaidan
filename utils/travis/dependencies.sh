#!/bin/bash -e
. utils/travis/common.sh

echo "*****************************************"
echo "Fetching dependencies"
! [ -z ${BUILD_VARIANT} ] && echo "Building ${BUILD_VARIANT}"
! [ -z ${BUILD_SYSTEM} ] && echo "Using build system: ${BUILD_SYSTEM}"
echo "*****************************************"

if [ ${TRAVIS_OS_NAME} == "linux" ] && ! [ ${BUILD_VARIANT} == "click" ]; then
	install_linux_deps
elif [ ${BUILD_VARIANT} == "click" ]; then
	install_click_deps
else
	echo "Can't build on \"${TRAVIS_OS_NAME}\""
	exit 1
fi
