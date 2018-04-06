# Kaidan - A user-friendly XMPP client for every device!

[![Build Status](https://travis-ci.org/KaidanIM/Kaidan.svg?branch=master)](https://travis-ci.org/KaidanIM/Kaidan)
[![Plasma Mobile CI Build Status](https://img.shields.io/jenkins/s/http/mobile.neon.pangea.pub:8080/job/xenial_unstable_KaidanIM_kaidan-packaging-deb.svg)](http://mobile.neon.pangea.pub:8080/job/xenial_unstable_KaidanIM_kaidan-packaging-deb)
[![Release](https://img.shields.io/github/release/kaidanim/kaidan.svg)](https://github.com/kaidanim/kaidan/releases)
[![Downloads](https://img.shields.io/github/downloads/kaidanim/kaidan/total.svg)](https://github.com/kaidanim/kaidan/releases)
[![Issues](https://img.shields.io/github/issues/kaidanim/kaidan.svg)](https://github.com/kaidanim/kaidan/issues)
[![Translation](https://hosted.weblate.org/widgets/kaidan/-/svg-badge.svg)](https://hosted.weblate.org/projects/kaidan/translations/)
[![license](https://img.shields.io/badge/License-GPLv3%2B%20%2F%20CC%20BY--SA%204.0-blue.svg)](https://raw.githubusercontent.com/kaidanim/kaidan/master/LICENSE)

![](https://user-images.githubusercontent.com/13557034/34342627-3f82443e-e9b6-11e7-9898-05205e9e9f9b.png)

## About

Kaidan is a simple, user-friendly Jabber/XMPP client providing a modern user-interface using
[Kirigami](https://techbase.kde.org/Kirigami) and [QtQuick](http://wiki.qt.io/Qt_Quick). The back-end of
Kaidan is entirely written in C++ using the [gloox](https://camaya.net/gloox/) XMPP client library and
[Qt 5](https://www.qt.io/).

Currently Kaidan only has been tested on GNU/Linux Desktops, Android & Plasma Mobile, but of course
Ubuntu Touch, OS X, iOS and Windows will follow. Of course this is still not everything, i.e. Sailfish OS
is missing. To support that we need to wait for a proper QtQuickControls 2 style for it or rewrite the GUI
using Silica. Both options are rather unlikely in the near future, so if you want to get a client for
Sailfish OS _now_, maybe take a look at [Shmoose](https://github.com/geobra/harbour-shmoose), a fork by an
earlier developer of Kaidan.

Kaidan is *not* finished yet, so don't expect it working as well as a finished client will do.

For a list of supported XEPs, have a look at the [Wiki](https://github.com/KaidanIM/Kaidan/wiki/Supported-XMPP-Extension-Protocols-(XEPs)).

## Compilation & Installation

* [Debian sid based systems](#debian-sid-based-systems)
* [Linux Desktop](#compile-kaidan-for-desktop-linux)
* [Ubuntu Touch (click package)](#compile-kaidan-for-ubuntu-touch)


### Debian sid based systems

Add our APT repository to your system

```
sudo echo "deb https://www.kaidan.im/packages sid main" > /etc/apt/sources.list.d/kaidan.list
wget https://www.kaidan.im/packages/pub.asc -qO - | sudo apt-key add -
sudo apt update
```

Then you can install Kaidan

```bash
sudo apt install kaidan
```

### Compile Kaidan for Desktop Linux

#### Install all dependencies:
 * [Qt](http://doc.qt.io/qt-5/build-sources.html) (Core Qml Quick Sql QuickControls2) (>= 5.8.0)
 * [gloox](https://camaya.net/gloox/download/) (>= 1.0.19)
 * [Kirigami 2](https://phabricator.kde.org/source/kirigami/) (>= 5.42.0)

Build only dependencies:
 * A C++ compiler
 * Qt Tools
 * CMake
 * Git (if you don't want to use tarballs/zips from github)

##### On Ubuntu *18.04+*, KDE Neon 16.04 or debian *sid* do the following:

Debian strech and older Ubuntu distros are not supported since they don't provide Qt 5.8 or newer.

```bash
sudo apt install apt-transport-https git-core cmake build-essential zlib1g-dev \
  libgloox-dev libnotify-bin libqt5quick5 libqt5quickcontrols2-5 libqt5quickwidgets5 \
  libqt5qml5 libqt5gui5 libqt5core5a qtdeclarative5-dev qttools5-dev qt5-default \
  qtquickcontrols2-5-dev kirigami2-dev
```

#### Get the source code:

```bash
git clone https://github.com/kaidanim/kaidan
```

#### Compile it:

```bash
mkdir kaidan/build ; cd kaidan/build
cmake .. -DI18N=1
make -j$(nproc)
```

#### Now you can run Kaidan:

```bash
./kaidan
```

#### If you like it, install it by:

```bash
sudo make install
```

### Compile Kaidan for Ubuntu Touch

This will fully automatically compile and build a click packge for Ubuntu Touch 16.04. Clickable and our
build script do the hard work for you, so building is rather simple.

Build-dependencies:
 * [Clickable](https://github.com/bhdouglass/clickable)
 * [Docker](https://www.docker.com/)
 * Git

#### Set up clickable docker container

After you've installed the dependencies, you need to set up the Ubuntu Touch SDK docker container.

```bash
clickable setup-docker
```

#### Clone Kaidan and start build

Now, you can clone our Kaidan repository using git and start building kaidan using clickable.

```bash
git clone https://github.com/kaidanim/kaidan
cd kaidan
clickable clean build click-build
```

The built click package will be located at `bin/ubuntu-touch/`.

----

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)
![CC BY-SA 4.0](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)
