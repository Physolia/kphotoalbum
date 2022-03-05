/* SPDX-FileCopyrightText: 2014-2022 Jesper K. Pedersen <blackie@kde.org>

   SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtMultimedia 5.15
import KPhotoAlbum 1.0
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

VideoOutput {
    id: root
    property alias imageId: remoteVideo.imageId
    property alias active : remoteVideo.active
    readonly property bool isLoading: media.status === MediaPlayer.NoMedia || media.status === MediaPlayer.Loading
    source: media

    RemoteVideoInfo {
        id: remoteVideo
    }

    MediaPlayer {
        id: media
        source: root.active ? remoteVideo.url : ""
        autoPlay: true
        notifyInterval: 250
        onError: console.log("Error from Media Player:" + errorString)
        onPlaying: _slideShow.videoRunning = true
        onStopped: _slideShow.videoRunning = false
    }

    MouseArea {
        anchors.centerIn: parent
        width: parent.width * 2/5
        height: parent.height * 2/5

        onClicked: {
            if (media.playbackState === MediaPlayer.PlayingState)
                media.pause()
            else
                media.play()
        }
    }

    ProgressBar {
        anchors.centerIn: parent
        value: remoteVideo.progress
        visible: root.active && value != 1
    }

    RowLayout {
        anchors {
            left: parent.left
            right: parent.right
            bottom: parent.bottom

            bottomMargin: _screenInfo.pixelForSizeInMM(2).height
        }
        visible: media.duration > 0

        TimeDisplay {
            milliseconds: media.position
            MouseArea {
                anchors.fill: parent
                onClicked: _settings.showVideoPlaybackSlider = !_settings.showVideoPlaybackSlider
            }
        }

        Slider {
            visible: _settings.showVideoPlaybackSlider
            // FIXME when portait image is shown in landscape, the slider is scaled down in size.
            handle.height: implicitHandleHeight * 2

            to: media.duration
            value: media.position

            onMoved: media.seek(value)

            RowLayout.fillWidth: true
        }

        TimeDisplay {
            milliseconds: media.duration
            visible: _settings.showVideoPlaybackSlider
        }

    }
    OnePixelTimeDisplay {
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: parent.right
        }
        height: _screenInfo.dotsPerMM * 1
        visible: !_settings.showVideoPlaybackSlider
        current: media.position
        max: media.duration
    }
}
