# Kaidan - A user-friendly XMPP client for every device!

[![Build Status](https://travis-ci.org/KaidanIM/Kaidan.svg?branch=master)](https://travis-ci.org/KaidanIM/Kaidan)
[![Release](https://img.shields.io/github/release/kaidanim/kaidan.svg)](https://github.com/kaidanim/kaidan/releases)
[![Downloads](https://img.shields.io/github/downloads/kaidanim/kaidan/total.svg)](https://github.com/kaidanim/kaidan/releases)
[![Issues](https://img.shields.io/github/issues/kaidanim/kaidan.svg)](https://github.com/kaidanim/kaidan/issues)
[![Translation](https://hosted.weblate.org/widgets/kaidan/-/svg-badge.svg)](https://hosted.weblate.org/projects/kaidan/translations/)
[![License](https://img.shields.io/badge/License-GPLv3%2B%20%2F%20CC%20BY--SA%204.0-blue.svg)](https://raw.githubusercontent.com/kaidanim/kaidan/master/LICENSE.txt)

![](http://i.imgur.com/k1Aup3h.png)

## About

Kaidan is a simple, user-friendly Jabber/XMPP client providing a modern user-interface
using [Kirigami](https://techbase.kde.org/Kirigami) and [QtQuick](http://wiki.qt.io/Qt_Quick).
The back-end of Kaidan is completely written in C++ using the [gloox](https://camaya.net/gloox/)
XMPP client library and [Qt 5](https://www.qt.io/).

Currently Kaidan only has been tested on GNU/Linux Desktops, but because of the use of Qt we shouldn't have
any problems porting it to all other common platforms, even Android. Nevertheless we will work on another
GUI using Silica for better look & feel on Sailfish OS soon™ (See [KaidanSF](https://github.com/KaidanIM/KaidanSF)).
But if you want to get a client for Sailfish OS _now_, maybe take a look at
[Shmoose](https://github.com/geobra/harbour-shmoose), a fork by an earlier developer of Kaidan.

Kaidan is *not* finished yet, so don't expect it working as well as a finished client will do.

## Screenshots

![](http://i.imgur.com/Yfuf6AT.png)

## Installation

### Debian Stretch based systems

Add our APT repository to your system

```
sudo apt install apt-transport-https software-properties-common dirmngr gnupg
sudo apt-add-repository "deb https://raw.githubusercontent.com/kaidanim/packages/master stretch main"
sudo apt-key adv --recv-key "210EB0BCA70439F0"
sudo apt update
```

Then you can install Kaidan

```bash
sudo apt install kaidan
```

### Compile it yourself

#### Install all dependencies:
 * [Qt](http://doc.qt.io/qt-5/build-sources.html) (Core Qml Quick Sql QuickControls2) (>= 5.8.0)
 * [gloox](https://camaya.net/gloox/download/) (>= 1.0.10)
 * [Kirigami 2](https://phabricator.kde.org/source/kirigami/) (>= 2.0.0)

Build only dependencies:
 * A C++ compiler
 * Qt Tools
 * CMake
 * Git (if you don't want to use tarballs/zips from github)

##### On Ubuntu *17.10+* or debian *sid* do the following:

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

----

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)
![CC BY-SA 4.0](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)
