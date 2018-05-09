#!/bin/bash -e
. utils/travis/common.sh

if [ $PLATFORM != "ubuntu-touch" ]; then
	sudo apt update
fi

echo "*****************************************"
echo "Fetching dependencies"
echo "^^^^^^^^^^^^^^^^^^^^^"
echo_env
echo "*****************************************"

if [ ${PLATFORM} == "linux-desktop" ]; then
	install_linux-desktop_deps
elif [ ${PLATFORM} == "ubuntu-touch" ]; then
	install_ubuntu-touch_deps
else
	echo "Can't build on \"${TRAVIS_OS_NAME}\""
	exit 1
fi
