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
            id: currentWeatherInfo
            width: parent.width
            height: parent.height / 2
            color: "#999999"

            RowLayout {
                id: currentWeatherDisplay
                width: currentWeatherInfo.width
                height: currentWeatherInfo.height
                spacing: 10 // Add spacing between the left and right parts

                ColumnLayout {
                    Layout.alignment: Qt.AlignTop | Qt.AlignLeft
                    Layout.leftMargin: 30
                    Layout.topMargin: 30
                    spacing: 20

                    Label {
                        text: propModel.currentCityName
                        font.pixelSize: 32
                    }
                    Label {
                        text: "Temperature: " + propModel.currentMainTemp + " °C"
                        font.pixelSize: 16
                    }
                    Label {
                        text: "Wind speed: " + propModel.currentWindSpeed + " m/s"
                        font.pixelSize: 16
                    }
                    Label {
                        text: "Rain: " + Math.floor(propModel.currentPop * 100) + " %"
                        font.pixelSize: 16
                    }
                }
                ColumnLayout {
                    Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                    spacing: 10

                    Image {
                        id: currentWeatherIconImage
                        source: "https://openweathermap.org/img/wn/" + propModel.currentWeatherIcon + "@2x.png"

                        // Handle errors or loading events
                        onStatusChanged: {
                            if (currentWeatherIconImage.status === Image.Error) {
                                console.error("Error loading weather icon:", currentWeatherIconImage.source)
                            } else if (currentWeatherIconImage.status === Image.Loading) {
                                console.log("Loading weather icon:", currentWeatherIconImage.source)
                            }
                        }
                    }
                    Label {
                        text: propModel.currentWeatherDescription
                        font.pixelSize: 16
                    }
                }

            }
        }

        // Lower section for forecast list
        Rectangle {
            id: forecastDisplay
            width: parent.width
            height: parent.height / 2  // Use the remaining half of the parent's height
            color: "#808080"

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
                            Layout.alignment: Qt.AlignCenter
                            Layout.topMargin: 10 // Add a margin to the top
                        }
                        Label {
                            text: Qt.formatDateTime(dateAndTime, "hh:mm")
                            font.pixelSize: 16
                            Layout.alignment: Qt.AlignCenter
                        }
                        Image {
                            id: weatherIconImage
                            // width: 50
                            // height: 50
                            Layout.alignment: Qt.AlignCenter
                            source: "https://openweathermap.org/img/wn/" + weatherIcon + "@2x.png"

                            // Handle errors or loading events
                            onStatusChanged: {
                                if (weatherIconImage.status === Image.Error) {
                                    console.error("Error loading weather icon:", weatherIconImage.source)
                                } else if (weatherIconImage.status === Image.Loading) {
                                    console.log("Loading weather icon:", weatherIconImage.source)
                                }
                            }
                        }
                        Label {
                            text: mainTemp + " °C"
                            font.pixelSize: 16
                            Layout.alignment: Qt.AlignCenter
                        }
                        Label {
                            text: "Rain: " + Math.floor(pop * 100) + " %"
                            font.pixelSize: 16
                            Layout.alignment: Qt.AlignCenter
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
