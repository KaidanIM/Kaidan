#!/bin/bash -e
. utils/travis/common.sh

echo "*****************************************"
echo "Fetching dependencies"
echo "Using build system: ${BUILD_SYSTEM}"
echo "*****************************************"

if [[ ${TRAVIS_OS_NAME} == "linux" ]]; then
	install_linux_deps
else
	echo "Can't build on \"${TRAVIS_OS_NAME}\""
	exit 1
fi
