/*
 *  Kaidan - A user-friendly XMPP client for every device!
 *
 *  Copyright (C) 2016-2019 Kaidan developers and contributors
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

import QtQuick 2.6
import QtQuick.Layouts 1.3
import QtQuick.Controls 2.4 as Controls
import QtMultimedia 5.10 as Multimedia
import org.kde.kirigami 2.2 as Kirigami

import im.kaidan.kaidan 1.0
import MediaUtils 0.1

Kirigami.Page {
	id: root

	title: qsTr("Multimedia Settings")
	topPadding: 0
	rightPadding: 0
	bottomPadding: 0
	leftPadding: 0

	Timer {
		id: pageTimer
		interval: 10

		onTriggered: {
			if (!root.isCurrentPage) {
				// Close the current page if it's not the current one after 10ms
				pageStack.pop()
			}

			// Stop the timer regardless of whether the page was closed or not
			pageTimer.stop()
		}
	}

	onIsCurrentPageChanged: {
		/*
		 * Start the timer if we are getting or loosing focus.
		 * Probably due to some kind of animation, isCurrentPage changes a few ms after
		 * this has been triggered.
		 */
		pageTimer.start()
	}

	MediaRecorder {
		id: recorder
	}

	ColumnLayout {
		id: mainLayout

		anchors {
			fill: parent
			leftMargin: 20
			topMargin: 5
			rightMargin: 20
			bottomMargin: 5
		}

		Controls.Label {
			text: qsTr('Configure')

			Layout.fillWidth: true
		}

		Controls.ComboBox {
			id: recorderTypesComboBox

			model: ListModel {
				ListElement {
					label: qsTr('Image Capture')
					type: MediaRecorder.Type.Image
				}

				ListElement {
					label: qsTr('Audio Recording')
					type: MediaRecorder.Type.Audio
				}

				ListElement {
					label: qsTr('Video Recording')
					type: MediaRecorder.Type.Video
				}
			}

			currentIndex: {
				switch (recorder.type) {
				case MediaRecorder.Type.Invalid:
					break
				case MediaRecorder.Type.Image:
					return 0
				case MediaRecorder.Type.Audio:
					return 1
				case MediaRecorder.Type.Video:
					return 2
				}

				return -1
			}

			textRole: 'label'
			delegate: Controls.RadioDelegate {
				text: model.label
				width: recorderTypesComboBox.width
				highlighted: recorderTypesComboBox.highlightedIndex === model.index
				checked: recorderTypesComboBox.currentIndex === model.index
			}

			Layout.fillWidth: true

			onActivated: {
				recorder.type = model.get(index).type
				_updateTabs()
			}

			onCurrentIndexChanged: {
				_updateTabs()
			}

			function _updateTabs() {
				for (var i = 0; i < bar.contentChildren.length; ++i) {
					if (bar.contentChildren[i].enabled) {
						bar.currentIndex = i
						return
					}
				}
			}
		}

		Controls.Label {
			id: cameraLabel

			text: qsTr('Camera')
			visible: recorder.type === MediaRecorder.Type.Image
					 || recorder.type === MediaRecorder.Type.Video
		}

		Controls.ComboBox {
			id: camerasComboBox

			visible: cameraLabel.visible
			model: recorder.cameraModel
			currentIndex: model.currentIndex
			displayText: model.currentCamera.description
			delegate: Controls.RadioDelegate {
				text: model.description
				width: camerasComboBox.width
				highlighted: camerasComboBox.highlightedIndex === model.index
				checked: camerasComboBox.currentIndex === model.index
			}

			Layout.fillWidth: true

			onActivated: {
				recorder.mediaSettings.camera = model.camera(index)
			}
		}

		Controls.Label {
			id: audioInputLabel

			text: qsTr('Audio input')
			visible: recorder.type === MediaRecorder.Type.Audio

			Layout.fillWidth: true
		}

		Controls.ComboBox {
			id: audioInputsComboBox

			visible: audioInputLabel.visible
			model: recorder.audioDeviceModel
			currentIndex: model.currentIndex
			displayText: model.currentAudioDevice.description
			delegate: Controls.RadioDelegate {
				text: model.description
				width: audioInputsComboBox.width
				highlighted: audioInputsComboBox.highlightedIndex === model.index
				checked: audioInputsComboBox.currentIndex === model.index
			}

			Layout.fillWidth: true

			onActivated: {
				recorder.mediaSettings.audioInputDevice = model.audioDevice(index)
			}
		}

		Controls.Label {
			id: containerLabel

			text: qsTr('Container')
			visible: recorder.type === MediaRecorder.Type.Audio
					 || recorder.type === MediaRecorder.Type.Video

			Layout.fillWidth: true
		}

		Controls.ComboBox {
			id: containersComboBox

			visible: containerLabel.visible
			model: recorder.containerModel
			currentIndex: model ? model.currentIndex : -1
			displayText: model ? model.currentDescription : ''
			delegate: Controls.RadioDelegate {
				text: model.description
				width: containersComboBox.width
				highlighted: containersComboBox.highlightedIndex === model.index
				checked: containersComboBox.currentIndex === model.index
			}

			Layout.fillWidth: true

			onActivated: {
				recorder.mediaSettings.container = model.value(index)
			}
		}

		Item {
			Layout.preferredHeight: parent.spacing * 2
			Layout.fillWidth: true
		}

		Controls.TabBar {
			id: bar

			Layout.fillWidth: true

			Controls.TabButton {
				text: qsTr('Image')
				enabled: recorder.type === MediaRecorder.Type.Image
			}

			Controls.TabButton {
				text: qsTr('Audio')
				enabled: recorder.type === MediaRecorder.Type.Audio
						 || recorder.type === MediaRecorder.Type.Video
			}

			Controls.TabButton {
				text: qsTr('Video')
				enabled: recorder.type === MediaRecorder.Type.Video
			}
		}

		Controls.ScrollView {
			id: scrollView

			Layout.fillWidth: true
			Layout.leftMargin: 5
			Layout.rightMargin: 5

			Controls.SwipeView {
				id: swipeView

				implicitWidth: scrollView.width
				currentIndex: bar.currentIndex
				enabled: recorder.isReady
				interactive: false
				clip: true

				ColumnLayout {
					id: imageTab

					enabled: bar.contentChildren[Controls.SwipeView.index].enabled

					Controls.Label {
						text: qsTr('Codec')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: imageCodecsComboBox

						model: recorder.imageCodecModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: imageCodecsComboBox.width
							highlighted: imageCodecsComboBox.highlightedIndex === model.index
							checked: imageCodecsComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.imageEncoderSettings.codec = model.value(index)
						}
					}

					Controls.Label {
						text: qsTr('Resolution')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: imageResolutionsComboBox

						model: recorder.imageResolutionModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: imageResolutionsComboBox.width
							highlighted: imageResolutionsComboBox.highlightedIndex === model.index
							checked: imageResolutionsComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.imageEncoderSettings.resolution = model.value(index)
						}
					}

					Controls.Label {
						text: qsTr('Quality')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: imageQualitiesComboBox

						model: recorder.imageQualityModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: imageQualitiesComboBox.width
							highlighted: imageQualitiesComboBox.highlightedIndex === model.index
							checked: imageQualitiesComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.imageEncoderSettings.quality = model.value(index)
						}
					}
				}

				ColumnLayout {
					id: audioTab

					enabled: bar.contentChildren[Controls.SwipeView.index].enabled

					Controls.Label {
						text: qsTr('Codec')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: audioCodecsComboBox

						model: recorder.audioCodecModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: audioCodecsComboBox.width
							highlighted: audioCodecsComboBox.highlightedIndex === model.index
							checked: audioCodecsComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.audioEncoderSettings.codec = model.value(index)
						}
					}

					Controls.Label {
						text: qsTr('Sample Rate')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: audioSampleRatesComboBox

						model: recorder.audioSampleRateModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: audioSampleRatesComboBox.width
							highlighted: audioSampleRatesComboBox.highlightedIndex === model.index
							checked: audioSampleRatesComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.audioEncoderSettings.sampleRate = model.value(index)
						}
					}

					Controls.Label {
						text: qsTr('Quality')

						Layout.fillHeight: true
					}

					Controls.ComboBox {
						id: audioQualitiesComboBox

						model: recorder.audioQualityModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: audioQualitiesComboBox.width
							highlighted: audioQualitiesComboBox.highlightedIndex === model.index
							checked: audioQualitiesComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.audioEncoderSettings.quality = model.value(index)
						}
					}
				}

				ColumnLayout {
					id: videoTab

					enabled: bar.contentChildren[Controls.SwipeView.index].enabled

					Controls.Label {
						text: qsTr('Codec')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: videoCodecsComboBox

						model: recorder.videoCodecModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: videoCodecsComboBox.width
							highlighted: videoCodecsComboBox.highlightedIndex === model.index
							checked: videoCodecsComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.videoEncoderSettings.codec = model.value(index)
						}
					}

					Controls.Label {
						text: qsTr('Resolution')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: videoResolutionsComboBox

						model: recorder.videoResolutionModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: videoResolutionsComboBox.width
							highlighted: videoResolutionsComboBox.highlightedIndex === model.index
							checked: videoResolutionsComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.videoEncoderSettings.resolution = model.value(index)
						}
					}

					Controls.Label {
						text: qsTr('Frame Rate')

						Layout.fillWidth: true
					}

					Controls.ComboBox {
						id: videoFrameRatesComboBox

						model: recorder.videoFrameRateModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: videoFrameRatesComboBox.width
							highlighted: videoFrameRatesComboBox.highlightedIndex === model.index
							checked: videoFrameRatesComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.videoEncoderSettings.frameRate = model.value(index)
						}
					}

					Controls.Label {
						text: qsTr('Quality')

						Layout.fillHeight: true
					}

					Controls.ComboBox {
						id: videoQualitiesComboBox

						model: recorder.videoQualityModel
						currentIndex: model.currentIndex
						displayText: model.currentDescription
						delegate: Controls.RadioDelegate {
							text: model.description
							width: videoQualitiesComboBox.width
							highlighted: videoQualitiesComboBox.highlightedIndex === model.index
							checked: videoQualitiesComboBox.currentIndex === model.index
						}

						Layout.fillWidth: true

						onActivated: {
							recorder.videoEncoderSettings.quality = model.value(index)
						}
					}
				}
			}
		}

		Item {
			Layout.fillWidth: true
			Layout.fillHeight: true

			Multimedia.VideoOutput {
				source: recorder

				width: sourceRect.width < parent.width && sourceRect.height < parent.height
							   ? sourceRect.width
							   : parent.width
				height: sourceRect.width < parent.width && sourceRect.height < parent.height
							   ? sourceRect.height
							   : parent.height

				anchors {
					centerIn: parent
				}
			}
		}

		Controls.Label {
			text: recorder.errorString
			visible: text !== ''
			color: 'red'

			font {
				bold: true
			}

			Layout.fillWidth: true
		}

		RowLayout {
			Controls.Label {
				text: {
					if (recorder.type === MediaRecorder.Type.Image) {
						return recorder.isReady ? qsTr('Ready') : qsTr('Initializing...')
					}

					switch (recorder.status) {
					case MediaRecorder.Status.UnavailableStatus:
						return qsTr('Unavailable')
					case MediaRecorder.Status.UnloadedStatus:
					case MediaRecorder.Status.LoadingStatus:
					case MediaRecorder.Status.LoadedStatus:
						return recorder.isReady ? qsTr('Ready') : qsTr('Initializing...')
					case MediaRecorder.Status.StartingStatus:
					case MediaRecorder.Status.RecordingStatus:
					case MediaRecorder.Status.FinalizingStatus:
						return qsTr('Recording...')
					case MediaRecorder.Status.PausedStatus:
						return qsTr('Paused')
					}
				}
			}

			Controls.DialogButtonBox {
				standardButtons: Controls.DialogButtonBox.RestoreDefaults
								 | Controls.DialogButtonBox.Reset
								 | Controls.DialogButtonBox.Save

				Layout.fillWidth: true

				onClicked: {
					if (button === standardButton(Controls.DialogButtonBox.RestoreDefaults)) {
						recorder.resetSettingsToDefaults()
					} else if (button === standardButton(Controls.DialogButtonBox.Reset)) {
						recorder.resetUserSettings()
					} else if (button === standardButton(Controls.DialogButtonBox.Save)) {
						recorder.saveUserSettings()
					}
				}
			}
		}
	}

	Component.onCompleted: {
		recorder.type = MediaRecorder.Type.Image
	}
}
