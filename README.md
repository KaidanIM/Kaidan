# Kaidan - A user-friendly XMPP client for every device!

[![Plasma Mobile CI Build Status](https://img.shields.io/jenkins/s/http/mobile.neon.pangea.pub:8080/job/xenial_unstable_KaidanIM_kaidan-packaging-deb.svg)](http://mobile.neon.pangea.pub:8080/job/xenial_unstable_KaidanIM_kaidan-packaging-deb)
[![Translation](https://hosted.weblate.org/widgets/kaidan/-/svg-badge.svg)](https://hosted.weblate.org/projects/kaidan/translations/)
[![license](https://img.shields.io/badge/License-GPLv3%2B%20%2F%20CC%20BY--SA%204.0-blue.svg)](https://raw.githubusercontent.com/kaidanim/kaidan/master/LICENSE)
[![Donations](https://img.shields.io/liberapay/patrons/kaidan.svg?logo=liberapay)](https://liberapay.com/kaidan)

![Kaidan v0.3 screenshot](https://git.kaidan.im/kaidan/kaidan/uploads/da4886ea50581517bde0278a804b311c/kaidan-screenshot-0.3.png)

## About

Kaidan is a simple, user-friendly Jabber/XMPP client providing a modern user-interface using
[Kirigami](https://techbase.kde.org/Kirigami) and [QtQuick](https://wiki.qt.io/Qt_Quick). The back-end of
Kaidan is entirely written in C++ using the [qxmpp](https://github.com/qxmpp-project/qxmpp) XMPP client library and
[Qt 5](https://www.qt.io/).

Currently Kaidan only has been tested on GNU/Linux Desktops, Android & Plasma Mobile, but of course
Ubuntu Touch, OS X, iOS and Windows will follow. Of course this is still not everything, i.e. Sailfish OS
is missing. To support that we need to wait for a proper QtQuickControls 2 style for it or rewrite the GUI
using Silica. Both options are rather unlikely in the near future, so if you want to get a client for
Sailfish OS _now_, maybe take a look at [Shmoose](https://github.com/geobra/harbour-shmoose), a fork by an
earlier developer of Kaidan.

Kaidan is *not* finished yet, so don't expect it working as well as a finished client will do.

For a list of supported XEPs, have a look at the [Wiki](https://git.kaidan.im/kaidan/kaidan/wikis/Supported-XEPs).

## Installing Kaidan

Instead of building Kaidan yourself, you can also just install ready-made builds. On these pages is explained
how to do that:
 * [Install the Flatpak](https://git.kaidan.im/kaidan/kaidan/wikis/install/flatpak)

## Building Kaidan

### Dependencies

Here are the general dependencies of Kaidan listed:
 * [Qt](https://doc.qt.io/qt-5/build-sources.html) (Core Qml Quick Svg Sql QuickControls2) (>= 5.8.0)
 * [qxmpp](https://github.com/qxmpp-project/qxmpp) (>= 0.9.0)
 * [Kirigami 2](https://phabricator.kde.org/source/kirigami/) (>= 5.42.0)
 * [ECM (extra-cmake-modules)](https://api.kde.org/ecm/manual/ecm.7.html)

### Build instructions

There are guides on how to build Kaidan for each supported platform (currently there are still pages missing):
 * [Linux Desktop](https://git.kaidan.im/kaidan/kaidan/wikis/building/linux-debian-based)
 * [Ubuntu Touch (click package)](https://git.kaidan.im/kaidan/kaidan/wikis/building/ubuntu-touch)
