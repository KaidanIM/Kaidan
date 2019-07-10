# Kaidan - A user-friendly XMPP client for every device!

[![Kaidan MUC](https://search.jabbercat.org/api/1.0/badge?address=kaidan@muc.kaidan.im)](https://i.kaidan.im)
[![Flatpak Build Status](https://binary-factory.kde.org/buildStatus/icon?job=Kaidan_flatpak)](https://binary-factory.kde.org/job/Kaidan_flatpak/)
[![Translation](https://hosted.weblate.org/widgets/kaidan/-/svg-badge.svg)](https://hosted.weblate.org/projects/kaidan/translations/)
[![license](https://img.shields.io/badge/License-GPLv3%2B%20%2F%20CC%20BY--SA%204.0-blue.svg)](https://raw.githubusercontent.com/kaidanim/kaidan/master/LICENSE)
[![Donations](https://img.shields.io/liberapay/patrons/kaidan.svg?logo=liberapay)](https://liberapay.com/kaidan)

![Kaidan screenshot](https://www.kaidan.im/images/screenshot.png)

## About

Kaidan is a simple, user-friendly Jabber/XMPP client providing a modern user
interface using [Kirigami][kg] and [QtQuick][qquick]. The back-end of Kaidan is
written in C++ using the [QXmpp][qxmpp] XMPP client library and [Qt 5][qt].

Kaidan runs on mobile and desktop Linux systems, including Plasma Mobile and
[Ubuntu Touch][openstore]. There are static Windows builds and macOS is also
supported, but we currently can't provide builds for it. The support for the
legacy mobile systems Android and iOS is very experimental and we currently only
provide test builds for [Android][android]. We might support SailfishOS in the
future in case the Sailfish SDK gets an update for Qt.

Kaidan is *not* finished yet, so don't expect it working as well as a finished
client will do -- Many features are still missing, many things still need to be
fixed and improved.

For a list of supported XEPs, have a look at the [Wiki][supp-xeps].

## Installing Kaidan

Instead of building Kaidan yourself, you can also just install ready-made builds. On these pages is explained
how to do that:
 * [Install the Flatpak](https://invent.kde.org/kde/kaidan/wikis/install/flatpak)
 * [Install from OpenStore (Ubuntu Touch)][openstore]

## Building Kaidan

### Dependencies

Here are the general dependencies of Kaidan listed:
 * [Qt](https://doc.qt.io/qt-5/build-sources.html) (Core Qml Quick Svg Sql QuickControls2) (>= 5.10.0)
 * [QXmpp][qxmpp] (>= 1.0.0)
 * [Kirigami 2](https://phabricator.kde.org/source/kirigami/) (>= 5.42.0)
 * [ECM (extra-cmake-modules)](https://api.kde.org/ecm/manual/ecm.7.html)
 * [KNotifications][knotif] (`-DUSE_KNOTIFICATIONS=OFF` to disable)

### Build instructions

There are guides on how to build Kaidan for each supported platform (currently there are still pages missing):
 * [Linux Desktop](https://invent.kde.org/kde/kaidan/wikis/building/linux-debian-based)
 * [Ubuntu Touch (click package)](https://invent.kde.org/kde/kaidan/wikis/building/ubuntu-touch)


[kg]: https://kde.org/products/kirigami/
[qquick]: https://wiki.qt.io/Qt_Quick
[qxmpp]: http://qxmpp.org
[qt]: https://www.qt.io/
[openstore]: https://open-store.io/app/im.kaidan.kaidan
[android]: https://www.kaidan.im/download/#android
[supp-xeps]: https://invent.kde.org/kde/kaidan/wikis/Supported-XEPs
[knotif]: https://api.kde.org/frameworks/knotifications/html/index.html
