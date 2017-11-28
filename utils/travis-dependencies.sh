#!/bin/bash

echo "*****************************************"
echo "Fetching dependencies"
echo "*****************************************"

apt update

apt install software-properties-common dirmngr gnupg ca-certificates -y

apt-key adv --recv-key "210EB0BCA70439F0"
echo deb https://raw.githubusercontent.com/kaidanim/packages/master sid main >> /etc/apt/sources.list

apt update
apt install cmake \
            build-essential \
            ninja-build \
            zlib1g-dev \
            libgloox-dev \
            libqt5quick5 \
            libqt5quickcontrols2-5 \
            libqt5quickwidgets5 \
            libqt5qml5 \
            libqt5gui5 \
            libqt5core5a \
            qtdeclarative5-dev \
            qttools5-dev \
            qt5-default \
            qtquickcontrols2-5-dev \
            kirigami2-dev -y
