/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2020 Kaidan developers and contributors
 *  (see the LICENSE file for a full list of copyright authors)
 *
 *  Kaidan is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  In addition, as a special exception, the author of Kaidan gives
 *  permission to link the code of its release with the OpenSSL
 *  project's "OpenSSL" library (or with modified versions of it that
 *  use the same license as the "OpenSSL" library), and distribute the
 *  linked executables. You must obey the GNU General Public License in
 *  all respects for all of the code used other than "OpenSSL". If you
 *  modify this file, you may extend this exception to your version of
 *  the file, but you are not obligated to do so.  If you do not wish to
 *  do so, delete this exception statement from your version.
 *
 *  Kaidan is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Kaidan.  If not, see <http://www.gnu.org/licenses/>.
 */

import QtQuick 2.7
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.3 as Controls
import QtPositioning 5.2 as Positioning

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

Loader {
	id: root

	property url mediaSource
	property int mediaSourceType
	property bool showOpenButton
	property QtObject message
	property QtObject mediaSheet

	enabled:  {
		switch (mediaSourceType) {
		case Enums.MessageType.MessageUnknown:
		case Enums.MessageType.MessageText:
		case Enums.MessageType.MessageFile:
		case Enums.MessageType.MessageDocument:
			return false;
		case Enums.MessageType.MessageImage:
		case Enums.MessageType.MessageAudio:
		case Enums.MessageType.MessageVideo:
		case Enums.MessageType.MessageGeoLocation:
			return mediaSheet
		}

	}
	visible: enabled
	sourceComponent: {
		switch (mediaSourceType) {
		case Enums.MessageType.MessageUnknown:
		case Enums.MessageType.MessageText:
		case Enums.MessageType.MessageFile:
		case Enums.MessageType.MessageDocument:
			return null
		case Enums.MessageType.MessageImage:
		case Enums.MessageType.MessageAudio:
		case Enums.MessageType.MessageVideo:
			return newMedia
		case Enums.MessageType.MessageGeoLocation:
			return newMediaLocation
		}
	}

	Layout.fillHeight: item ? item.Layout.fillHeight : false
	Layout.fillWidth: item ? item.Layout.fillWidth : false
	Layout.preferredHeight: item ? item.Layout.preferredHeight : -1
	Layout.preferredWidth: item ? item.Layout.preferredWidth : -1
	Layout.minimumHeight: item ? item.Layout.minimumHeight : -1
	Layout.minimumWidth: item ? item.Layout.minimumWidth : -1
	Layout.maximumHeight: item ? item.Layout.maximumHeight : -1
	Layout.maximumWidth: item ? item.Layout.maximumWidth : -1
	Layout.alignment: item ? item.Layout.alignment : Qt.AlignCenter
	Layout.margins: item ? item.Layout.margins : 0
	Layout.leftMargin: item ? item.Layout.leftMargin : 0
	Layout.topMargin: item ? item.Layout.topMargin : 0
	Layout.rightMargin: item ? item.Layout.rightMargin : 0
	Layout.bottomMargin: item ? item.Layout.bottomMargin : 0

	Component {
		id: newMedia

		NewMedia {
			mediaSourceType: root.mediaSourceType
			showOpenButton: root.showOpenButton
			message: root.message
			mediaSheet: root.mediaSheet

			onMediaSourceChanged: {
				root.mediaSheet.source = mediaSource
			}
		}
	}

	Component {
		id: newMediaLocation

		NewMediaLocation {
			mediaSourceType: root.mediaSourceType
			showOpenButton: root.showOpenButton
			message: root.message
			mediaSheet: root.mediaSheet

			onMediaSourceChanged: {
				root.mediaSheet.source = mediaSource
			}
		}
	}
}
