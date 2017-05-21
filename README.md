# Kaidan - A user-friendly XMPP client for every device!

[![Release](https://img.shields.io/github/release/kaidanim/kaidan.svg)](https://github.com/kaidanim/kaidan/releases)
[![Downloads](https://img.shields.io/github/downloads/kaidanim/kaidan/total.svg)](https://github.com/kaidanim/kaidan/releases)
[![Issues](https://img.shields.io/github/issues/kaidanim/kaidan.svg)](https://github.com/kaidanim/kaidan/issues)
[![Translation](https://hosted.weblate.org/widgets/kaidan/-/svg-badge.svg)](https://hosted.weblate.org/projects/kaidan/translations/)
[![License](https://img.shields.io/badge/License-GPLv3%2B%20%2F%20CC%20BY--SA%204.0-blue.svg)](https://raw.githubusercontent.com/kaidanim/kaidan/master/LICENSE.txt)


## About

Kaidan is a simple, user-friendly Jabber/XMPP client providing a modern user-interface
using [Kirigami](https://techbase.kde.org/Kirigami) and [QtQuick](http://wiki.qt.io/Qt_Quick).
The back-end of Kaidan is completely written in C++ using the [Swiften](http://swift.im/swiften.html)
Library of the [Swift Instant Messenger](http://swift.im/swift.html) and [Qt 5](https://www.qt.io/).

Currently Kaidan only has been tested on GNU/Linux Desktops, but because of the use of Qt we shouldn't have
any problems porting it to all other common platforms, even Android. Nevertheless we will work on another
GUI using Silica for better look & feel on Sailfish OS soon (See [KaidanSF](https://github.com/KaidanIM/KaidanSF)).
But if you want to get a client for Sailfish OS _now_, maybe take a look at
[Shmoose](https://github.com/geobra/harbour-shmoose), a fork by an earlier developer of Kaidan.

Kaidan is *not* finished yet, so don't expect it working as well as a finished client will do.


## Installation

### Debian Sid based systems

Note: We can not guarantee that the packages available from the repository are always up to date.

Add our apt repository to your system

 * `sudo apt-get install apt-transport-https software-properties-common`
 * `sudo apt-add-repository "deb https://raw.githubusercontent.com/kaidanim/packages/master sid main"`
 * `sudo apt-key adv --keyserver hkp://pgp.mit.edu --recv-key "210EB0BCA70439F0"`
 * `sudo apt-get update`

Then install Kaidan

 * `sudo apt-get install kaidan`

### Compile it yourself

On Linux do the following:

Install dependencies to build Kaidan (for Debian Stretch and Sid)

 *  `sudo apt-get install apt-transport-https git-core cmake build-essential zlib1g-dev libglib2.0-dev libssl-dev libxml2-dev libcrypto++-dev libpthread-stubs0-dev libidn11-dev libminiupnpc-dev libnatpmp-dev libswiften-dev libboost-system-dev libnotify-bin libboost-program-options-dev libboost-serialization-dev libqt5quick5 libqt5quickcontrols2-5 libqt5quickwidgets5 libqt5qml5 libqt5gui5 libqt5core5a qtdeclarative5-dev qttools5-dev qt5-default qml-module-qtquick-controls2`

Debian currently has no Kirigami2 packages, so we have to install them from external sources.

 * `sudo apt-add-repository "deb https://raw.githubusercontent.com/kaidanim/packages/master sid main"`
 * `sudo apt-key adv --keyserver hkp://pgp.mit.edu --recv-key "210EB0BCA70439F0"`
 * `sudo apt-get update`
 * `sudo apt-get install kirigami2-dev`

Get Kaidan source code

 * `git clone https://github.com/kaidanim/kaidan`

Finally compile it

 * `mkdir kaidan/build ; cd kaidan/build`
 * `cmake .. -DI18N=1`
 * `make -j<number of threads>`

Now you can run Kaidan:

 * `./kaidan`

If you like it, install it by:

 * `sudo make install`

----

![GPLv3](https://www.gnu.org/graphics/gplv3-127x51.png)
![CC BY-SA 4.0](https://i.creativecommons.org/l/by-sa/4.0/88x31.png)
