import QtQuick 2.9
import QtQuick.Window 2.2
import QtQuick.Layouts 1.3
import QtQuick.Controls 1.2

Window {
    id: window
    visible: true
    visibility: "FullScreen"
    width: 800
    height: 480
    title: qsTr("pygiri")
    color: "#181A1F"

    Item {
        anchors.fill: parent
        focus: true
        Keys.onPressed: {
            if (event.key === Qt.Key_1) {
                console.log("pressed key", event.key-48)
                exe.counter_add(1)
            }
            if (event.key === Qt.Key_2) {
                console.log("pressed key", event.key-48)
                exe.timer_reset()
            }
            if (event.key === Qt.Key_3) {
                console.log("pressed key", event.key-48)
                exe.timer_start()
            }
            if (event.key === Qt.Key_4) {
                console.log("pressed key", event.key-48)
                exe.timer_pause()
            }
            if (event.key === Qt.Key_5) {
                console.log("pressed key", event.key-48)
                rectangleName.visible = false
            }
            if (event.key === Qt.Key_6) {
                console.log("pressed key", event.key-48)
                rectangleName.visible = true
            }
            if (event.key === Qt.Key_7) {
                console.log("pressed key", event.key-48)
                exe.counter_add(-1)
            }
        }
    }

    FontLoader { id: localFont; source: "fonts/DS-DIGI.TTF" }
    ColumnLayout {
        id: columnLayout
        anchors.fill: parent
        anchors.margins: 20

        WhiteText {
            id: whiteTextName
            text: qsTr("СПОРТСМЕН");
            Layout.fillWidth: true
            visible: false
        }

        Rectangle {
            id: rectangleName
            Layout.fillWidth: true
            color: "#000000"
            Layout.minimumHeight: 150
            Layout.maximumHeight: 300
            visible: false

            Text {
                id: textName;
                width: parent.width;
                height: parent.height;
                color: "red";
                text: qsTr("---");
                verticalAlignment: Text.AlignVCenter;
                horizontalAlignment: Text.AlignHCenter;
                minimumPointSize: 10;
                font.pointSize: 300;
                fontSizeMode: Text.Fit
            }
        }
        RowLayout {
            spacing: 30
            Layout.fillWidth: true
            //transformOrigin: Item.Top


            ColumnLayout {
                id: nameColumnLayout

                Layout.minimumWidth: 400

                WhiteText {
                    text: qsTr("ВРЕМЯ")
                    Layout.fillWidth: true
                }

                Rectangle {
                    Layout.fillHeight: true
                    Layout.fillWidth: true
                    color: "#000000"
                    transformOrigin: Item.Center
                    Layout.minimumHeight: 150
                    //Layout.maximumHeight: 50
                    Text {
                        id: textStopwatch
                        width: parent.width
                        height: parent.height
                        color: "red"
                        font { family: localFont.name;  capitalization: Font.Capitalize }
                        text: qsTr("00:00")
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        minimumPointSize: 10
                        font.pointSize: 350
                        fontSizeMode: Text.Fit
                    }
                }
            }

            ColumnLayout {
                id: nameAttemptLayout
                Layout.minimumWidth: 300
                WhiteText {
                    text: qsTr("ПОДХОД")
                    Layout.fillWidth: true
                }

                Rectangle {
                    id: rectangleAttemptCount
                    color: "#000000"
                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    Layout.minimumHeight: 150
                    Text {
                        id: textCounter
                        width: parent.width
                        height: parent.height
                        color: "red"
                        font {
                            family: localFont.name;
                            //pixelSize: rectangleAttemptCount.font.pixelSize * 0.8;
                            capitalization: Font.Capitalize
                        }
                        text: qsTr("0")
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        minimumPointSize: 10
                        font.pointSize: 300
                        fontSizeMode: Text.Fit
                    }
                }
            }


        }
        Text {
            id: textPlatform
            text: qsTr("Номер платформы 0")
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignRight
            color: "#ffffff"
            //anchors.left: parent.left
            //anchors.bottom: parent.bottom
        }
    }

    Connections {
        target: exe

        onSetTimer: {
            textStopwatch.text = strtimer
        }

        onSetCounter: {
            textCounter.text = strcounter
        }

        onSetVisible: {
            if (intvisible === 1) {
                whiteTextName.visible = true
                rectangleName.visible = true
            } else {
                whiteTextName.visible = false
                rectangleName.visible = false
            }
        }

        onSetName: {
            textName.text = strname
        }
    }

}
