#!/bin/bash -e

export CPUS_USED=2
if command -v nproc >/dev/null; then
	export CPUS_USED=$(nproc)
fi

if [[ ${PLATFORM} == "ubuntu-touch" ]]; then
	export BUILD_SYSTEM="cmake"
elif [[ ${PLATFORM} == "" ]]; then
	# currently there's only linux-desktop & ut
	# otherwise other parameters (as TRAVIS_OS_NAME) could be checked
	export PLATFORM="linux-desktop"
fi

echo_env() {
	echo "PLATFORM=${PLATFORM}"
	echo "BUILD_SYSTEM=${BUILD_SYSTEM}"
	echo "CPUS_USED=${CPUS_USED}"
}

add_linux-desktop_apt_repos() {
	sudo apt-get install dirmngr

	# trusty backports
	sudo bash -c "echo deb http://archive.ubuntu.com/ubuntu trusty-backports main restricted universe multiverse >> /etc/apt/sources.list"

	# Qt 5.9 repository
	sudo add-apt-repository -y ppa:beineri/opt-qt-5.10.1-trusty
}

add_ubuntu-touch_apt_repos() {
	sudo add-apt-repository ppa:bhdouglass/clickable -y
}

install_kf5() {
	wget -c "https://github.com/JBBgameich/precompiled-kf5-linux/releases/download/KF5.40/kf5.40-gcc6-linux64-release.tar.xz"
	tar xf kf5.40-gcc6-linux64-release.tar.xz
	sudo cp -rf kf5-release/* /opt/qt*/
}

install_gloox() {
	# download
	wget -c "https://github.com/JBBgameich/precompiled-kf5-linux/releases/download/KF5.40/libgloox17_1.0.20-1_amd64.deb"
	wget -c "https://github.com/JBBgameich/precompiled-kf5-linux/releases/download/KF5.40/libgloox-dev_1.0.20-1_amd64.deb"

	# install debs and install possible missing dependencies
	sudo dpkg -i libgloox*.deb || sudo apt-get -f -y install

	# clean up
	rm libgloox*.deb
}

install_linux-desktop_deps() {
	add_linux-desktop_apt_repos

	sudo apt-get update
	sudo apt-get install -y -t trusty-backports \
	                     cmake \
	                     build-essential \
	                     ninja-build \
	                     zlib1g-dev \
	                     qt510base \
	                     qt510script \
	                     qt510declarative \
	                     qt510tools \
	                     qt510x11extras \
	                     qt510svg \
	                     qt510quickcontrols2

	# KF5 (only for cmake; qmake compiles kirigami from submodule)
	if [[ $BUILD_SYSTEM == "cmake" ]]; then
		install_kf5
	fi
	install_gloox
}

install_ubuntu-touch_deps() {
	add_ubuntu-touch_apt_repos

	sudo apt-get update
	sudo apt-get install clickable
	clickable setup-docker
}

env_setup() {
	if [ -f /opt/qt5*/bin/qt5*-env.sh ]; then
		echo "I: Setting up custom Qt installation..."
		source /opt/qt5*/bin/qt5*-env.sh
	fi
}
