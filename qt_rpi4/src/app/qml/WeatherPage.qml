// WeatherPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.greenoasis.weather 1.0

Item {
    id: weatherPage
    width: 800
    height: 480

    property var model: null // The model property will be set from outside

    Row {
        anchors.fill: parent

        // Current weather display
        Rectangle {
            width: weatherPage.width / 2
            height: weatherPage.height
            color: "#a3c9f7"

            ListView {
                anchors.fill: parent
                model: model // Use the property 'model' directly
                delegate: Item {
                    width: parent.width
                    height: isCurrentWeather ? parent.height : 0
                    visible: isCurrentWeather

                    Column {
                        spacing: 10
                        anchors.centerIn: parent

                        Label {
                            text: "Temp: " + mainTemp + "°C"
                            font.pixelSize: 24
                        }
                        Label {
                            text: "Min Temp: " + mainTempMin + "°C"
                            font.pixelSize: 20
                        }
                        Label {
                            text: "Max Temp: " + mainTempMax + "°C"
                            font.pixelSize: 20
                        }
                    }
                }
            }
        }

        // Weather forecast display
        Rectangle {
            width: weatherPage.width / 2
            height: weatherPage.height
            color: "#cbe8fc"

            ListView {
                anchors.fill: parent
                model: model // Use the property 'model' directly
                delegate: Item {
                    width: parent.width
                    height: !isCurrentWeather ? 100 : 0
                    visible: !isCurrentWeather && index > 0

                    Column {
                        spacing: 10
                        anchors.centerIn: parent

                        Label {
                            text: "Temp: " + mainTemp + "°C"
                            font.pixelSize: 24
                        }
                        Label {
                            text: "Min Temp: " + mainTempMin + "°C"
                            font.pixelSize: 20
                        }
                        Label {
                            text: "Max Temp: " + mainTempMax + "°C"
                            font.pixelSize: 20
                        }
                    }
                }
            }
        }
    }
}
