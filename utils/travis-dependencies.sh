#!/bin/bash

echo "*****************************************"
echo "Fetching dependencies"
echo "*****************************************"

sudo apt update

sudo apt install software-properties-common dirmngr gnupg ca-certificates wget -y

# Trusty backports
sudo bash -c "echo deb http://archive.ubuntu.com/ubuntu trusty-backports main restricted universe multiverse >> /etc/apt/sources.list"

# Kaidan repository
sudo apt-key adv --recv-key "210EB0BCA70439F0"
sudo bash -c "echo deb https://raw.githubusercontent.com/kaidanim/packages/master sid main >> /etc/apt/sources.list"

# Qt 5.9 repository
sudo add-apt-repository ppa:beineri/opt-qt593-trusty -y

sudo apt update
sudo apt install cmake \
            build-essential \
            ninja-build \
            zlib1g-dev \
            libgloox-dev \
            qt59base \
            qt59script \
            qt59declarative \
            qt59tools \
            qt59x11extras \
            qt59svg \
            qt59quickcontrols2 \
            -y -t trusty-backports

# KF5
wget -c "https://github.com/JBBgameich/precompiled-kf5-linux/releases/download/KF5.40/kf5.40-gcc6-linux64-release.tar.xz"
tar xf kf5.40-gcc6-linux64-release.tar.xz
sudo cp -Rf kf5-release/* /opt/qt*/

# Gloox
wget -c "https://github.com/JBBgameich/precompiled-kf5-linux/releases/download/KF5.40/libgloox17_1.0.20-1_amd64.deb"
wget -c "https://github.com/JBBgameich/precompiled-kf5-linux/releases/download/KF5.40/libgloox-dev_1.0.20-1_amd64.deb"
sudo dpkg -i libgloox-*.deb
rm libgloox*.deb
sudo apt install -f -y
