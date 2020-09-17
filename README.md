# Kaidan - A user-friendly XMPP client for every device!

[![Kaidan MUC](https://search.jabbercat.org/api/1.0/badge?address=kaidan@muc.kaidan.im)](https://i.kaidan.im)
[![Translation](https://hosted.weblate.org/widgets/kaidan/-/svg-badge.svg)](https://hosted.weblate.org/projects/kaidan/translations/)
[![license](https://img.shields.io/badge/License-GPLv3%2B%20%2F%20CC%20BY--SA%204.0-blue.svg)](https://raw.githubusercontent.com/kaidanim/kaidan/master/LICENSE)
[![Donations](https://img.shields.io/liberapay/patrons/kaidan.svg?logo=liberapay)](https://liberapay.com/kaidan)

[![Flatpak build status](https://binary-factory.kde.org/buildStatus/icon?subject=Flatpak&job=Kaidan_x86_64_flatpak)](https://binary-factory.kde.org/job/Kaidan_x86_64_flatpak/)
[![Android build status](https://binary-factory.kde.org/buildStatus/icon?subject=Android&job=Kaidan_android)](https://binary-factory.kde.org/job/Kaidan_android/)
[![Windows build status](https://binary-factory.kde.org/buildStatus/icon?subject=Windows&job=Kaidan_Nightly_win64)](https://binary-factory.kde.org/job/Kaidan_Nightly_win64/)
[![macOS build status](https://binary-factory.kde.org/buildStatus/icon?subject=macOS&job=Kaidan_Nightly_macos)](https://binary-factory.kde.org/job/Kaidan_Nightly_macos/)

![Kaidan screenshot](https://www.kaidan.im/images/screenshot.png)

## About

[Kaidan][kaidan-website] is a simple, user-friendly and modern chat client. It
uses the open communication protocol [XMPP (Jabber)][xmpp]. The user interface
makes use of [Kirigami][kirigami-website] and [QtQuick][qtquick], while the
back-end of Kaidan is entirely written in C++ using [Qt][qt] and the Qt-based
XMPP library [QXmpp][qxmpp].

Kaidan runs on mobile and desktop systems including Linux, Windows, macOS,
Android, Plasma Mobile and Ubuntu Touch.
Unfortunately, we are not able to provide builds for all platforms atthe moment
due to little developer resources.

Kaidan does *not* have all basic features yet and has still some stability
issues. Do not expect it to be as good as the currently dominating instant
messaging clients.

If you are interested in the technical features Kaidan supports, you can have a
look at our [overview of XEPs and RFCs][xeps-rfcs].

## Using and Building Kaidan

Downloadable builds are available on [Kaidan's download page][downloads].
Instructions for using ready-made (nightly / stable) builds and for building
Kaidan yourself can be found in our [Wiki][wiki].

## Dependencies

Here are the general dependencies of Kaidan:
 * [Qt][qt-build-sources] (Core Qml Quick Svg Sql QuickControls2 Xml Multimedia Positioning Location) (>= 5.12.0)
 * [QXmpp][qxmpp] (>= 1.2.0)
 * [Kirigami 2][kirigami-repo] (>= 5.67.0)
 * [ECM (extra-cmake-modules)][ecm]
 * [ZXing-cpp][zxing-cpp] (>= 1.0.8)
 * [KNotifications][knotifications] (`-DUSE_KNOTIFICATIONS=OFF` to disable)

## Contributing

If you are interested in contributing to Kaidan, please have a look at our
[contribution guidelines][contributing]. If you want to improve Kaidan's
website, feel free to visit its [project site][kaidan-website-repo].

[contributing]: CONTRIBUTING.md
[downloads]: https://www.kaidan.im/download/
[ecm]: https://api.kde.org/ecm/manual/ecm.7.html
[kaidan-website]: https://kaidan.im
[kaidan-website-repo]: https://invent.kde.org/websites/kaidan-im
[kirigami-repo]: https://invent.kde.org/frameworks/kirigami
[kirigami-website]: https://kde.org/products/kirigami/
[knotifications]: https://api.kde.org/frameworks/knotifications/html/index.html
[qt]: https://www.qt.io/
[qt-build-sources]: https://doc.qt.io/qt-5/build-sources.html
[qtquick]: https://wiki.qt.io/Qt_Quick
[qxmpp]: https://github.com/qxmpp-project/qxmpp
[wiki]: https://invent.kde.org/network/kaidan/-/wikis/home
[xeps-rfcs]: https://invent.kde.org/network/kaidan/-/wikis/xeps-rfcs
[xmpp]: https://xmpp.org
[zxing-cpp]: https://github.com/nu-book/zxing-cpp
