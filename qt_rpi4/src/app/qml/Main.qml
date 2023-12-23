import QtQuick
import QtQuick.Window

// Import C++ modules into QML
import com.greenoasis.weathermodel
import com.greenoasis.weatherdata

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

    WeatherPage{
        id: weatherPage
    }

}
