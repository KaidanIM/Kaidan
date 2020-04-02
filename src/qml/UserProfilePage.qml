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

import QtQuick 2.12
import QtQuick.Controls 2.12 as Controls
import QtQuick.Layouts 1.12
import QtQml 2.12
import org.kde.kirigami 2.12 as Kirigami

import im.kaidan.kaidan 1.0
import "elements"

Kirigami.Page {
	id: root
	title: qsTr("Profile")
	topPadding: 0
	rightPadding: 0
	bottomPadding: 0
	leftPadding: 0

	property string jid
	property string name

	Timer {
		id: pageTimer
		interval: 10

		onTriggered: {
			if (!root.isCurrentPage) {
				// Close the current page if it's not the current one after 10ms
				pageStack.pop();
			}

			// Stop the timer regardless of whether the page was closed or not
			pageTimer.stop();
		}
	}

	QrCodeGenerator {
		id: qrCodeGenerator
	}

	onIsCurrentPageChanged: {
		/*
		 * Start the timer if we are getting or loosing focus.
		 * Probably due to some kind of animation, isCurrentPage changes a few ms after
		 * this has been triggered.
		 */
		pageTimer.start();
	}

	actions {
		left: Kirigami.Action {
			text: qsTr("Rename contact")
			icon.name: "document-edit-symbolic"

			onTriggered: {
				renameSheet.open()
				renameSheet.forceFocus()
			}
		}

		main: Kirigami.Action {
			text: qsTr("Show QR code")
			icon.name: "view-barcode-qr"
			onTriggered: qrCodeSheet.open()
		}

		right: Kirigami.Action {
			text: qsTr("Remove contact")
			icon.name: "edit-delete-symbolic"
			onTriggered: removeSheet.open()
		}
	}

	UserPresenceWatcher {
		id: userPresence
		jid: root.jid
	}

	RosterRenameContactSheet {
		id: renameSheet
		jid: root.jid
		enteredName: {
			if (name === jid)
				return "";
			return name;
		}
	}

	Kirigami.OverlaySheet {
		id: qrCodeSheet

		Kirigami.Icon {
			width: parent.width
			height: width
			source: width > 0 ? qrCodeGenerator.generateBareJidQrCode(width, jid) : ""
		}
	}

	RosterRemoveContactSheet {
		id: removeSheet
		jid: root.jid
	}

	Controls.ScrollView {
		anchors.fill: parent
		clip: true
		contentWidth: root.width
		contentHeight: content.height

		ColumnLayout {
			id: content
			x: 20
			y: 5
			width: root.width - 40

			Item {
				Layout.preferredHeight: 10
			}

			RowLayout {
				Layout.alignment: Qt.AlignTop
				Layout.fillWidth: true
				spacing: 20

				Avatar {
					Layout.preferredHeight: Kirigami.Units.gridUnit * 10
					Layout.preferredWidth: Kirigami.Units.gridUnit * 10
					name: root.name
					avatarUrl: Kaidan.avatarStorage.getAvatarUrl(jid)
				}

				ColumnLayout {
					Kirigami.Heading {
						Layout.fillWidth: true
						text: root.name
						textFormat: Text.PlainText
						maximumLineCount: 2
						elide: Text.ElideRight
					}

					Controls.Label {
						text: root.jid
						color: Kirigami.Theme.disabledTextColor
						textFormat: Text.PlainText
					}

					RowLayout {
						spacing: Kirigami.Units.smallSpacing

						Kirigami.Icon {
							source: userPresence.availabilityIcon
							width: 26
							height: 26
						}

						Controls.Label {
							Layout.alignment: Qt.AlignVCenter
							text: userPresence.availabilityText
							color: userPresence.availabilityColor
							textFormat: Text.PlainText
						}

						Item {
							Layout.fillWidth: true
						}
					}
				}
			}

			Repeater {
				model: VCardModel {
					jid: root.jid
				}

				delegate: ColumnLayout {
					Layout.fillWidth: true

					Controls.Label {
						text: Utils.formatMessage(model.value)
						onLinkActivated: Qt.openUrlExternally(link)
						textFormat: Text.StyledText
					}

					Controls.Label {
						text: model.key
						color: Kirigami.Theme.disabledTextColor
						textFormat: Text.PlainText
					}

					Item {
						height: 3
					}
				}
			}

			// placeholder for left, right and main action
			Item {
				visible: Kirigami.Settings.isMobile
				Layout.preferredHeight: 60
			}
		}
	}
}
