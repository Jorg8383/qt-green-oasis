import QtQuick
import QtQuick.Window

Window {
    id: window
    width: 800
    height: 480
    visible: true
    title: qsTr("Hello World")

    Rectangle {
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter

        Text {
            id: name
            text: qsTr("Green Oasis Pi")
        }
    }
}
