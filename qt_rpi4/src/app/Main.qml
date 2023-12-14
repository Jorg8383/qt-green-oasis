import QtQuick
import QtQuick.Window

// Import C++ modules
import com.weather.weatherdata
import com.weather.weathermodel


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
