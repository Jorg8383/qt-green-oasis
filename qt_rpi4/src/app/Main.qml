import QtQuick
import QtQuick.Window

// import com.greenoasis.weathermodel



Window {
    id: window
    width: 800
    height: 480
    visible: true
    title: qsTr("Hello World")

    Rectangle {

        Text {
            id: name
            text: qsTr("Green Oasis Pi")
        }
    }

    // WeatherModel{}
}
