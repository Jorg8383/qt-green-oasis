// WeatherPage.qml
import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.greenoasis.weather 1.0

Item {
    id: weatherPage
    width: 800
    height: 480

    property var propModel: null // The model property will be set from outside

    Column {
        anchors.fill: parent

        // Upper section for current weather
        Rectangle {
            id: currentWeatherDisplay
            width: parent.width
            height: parent.height / 2  // Use half of the parent's height
            color: "#a3c9f7"

            // Assuming the first item in the model is the current weather
            // and has the 'isCurrentWeather' property set to true.
            // Label {
            //     text: "Current Temp: " + (propModel ? propModel.get(0).mainTemp : "") + "°C"
            //     font.pixelSize: 24
            //     anchors.centerIn: parent
            // }
            // Label {
            //     text: "Min Temp: " + propModel.get(0).mainTempMin + "°C"
            //     font.pixelSize: 20
            //     anchors { top: previousLabel.bottom; horizontalCenter: parent.horizontalCenter }
            // }
            // Label {
            //     text: "Max Temp: " + propModel.get(0).mainTempMax + "°C"
            //     font.pixelSize: 20
            //     anchors { top: previousLabel.bottom; horizontalCenter: parent.horizontalCenter }
            // }
        }

        // Lower section for forecast list
        Rectangle {
            id: forecastDisplay
            width: parent.width
            height: parent.height / 2  // Use the remaining half of the parent's height
            color: "#cbe8fc"

            ListView {
                anchors.fill: parent
                orientation: ListView.Horizontal // Set the ListView to horizontal orientation
                layoutDirection: Qt.LeftToRight
                model: propModel // Set your WeatherModel instance as the model
                clip: true // Clip the ListView's contents

                delegate: Item {
                    width: 200 // Set a fixed width for each list item
                    height: ListView.height // The height matches the ListView's height

                    // Check if the item is not the current weather
                    // visible: index > 0

                    ColumnLayout {
                        anchors.fill: parent
                        spacing: 10

                        Label {
                            text: Qt.formatDateTime(dateAndTime, "dddd")
                            font.pixelSize: 16
                        }
                        Label {
                            text: Qt.formatDateTime(dateAndTime, "hh:mm")
                            font.pixelSize: 16
                        }

                        Label {
                            text: mainTemp + "°C"
                            font.pixelSize: 24
                        }
                        Label {
                            text: "Min: " + mainTempMin + "°C"
                            font.pixelSize: 16
                        }
                        Label {
                            text: "Max: " + mainTempMax + "°C"
                            font.pixelSize: 16
                        }
                    }

                }
                // Scrollbar for the ListView
                ScrollBar.horizontal: ScrollBar {
                    id: horizontalScrollBar
                    active: true
                }

            }
        }
    }
}
