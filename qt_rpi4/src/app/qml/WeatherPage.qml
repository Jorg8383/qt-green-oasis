import QtQuick 2.15
import QtQuick.Controls

import com.greenoasis.weather 1.0
// import com.greenoasis.weatherdata

Item {
    id: weatherPage

    // For debugging purposes only!
    Component.onCompleted: {
        console.log("onCompleted - weatherModelCount:" + weatherModel.count)
    }

    // For debugging purposes only!
    Connections {
        target: weatherModel
        function onCountChanged(newCount) {
            console.log("QML - countChanged signal was received with new content:", newCount)
        }
    }

    Row {
        anchors.fill: parent
        spacing: 10

        // Current weather information
        Rectangle {
            id: currentWeather
            color: "lightblue"

            ListView {
                id: listViewCurrent
                model: weatherModel
                delegate: Rectangle {
                    visible: model.isCurrentWeather
                    width: listViewCurrent.width
                    height: visible ? 100 : 0
                    color: index % 2 == 0 ? "lightgray" : "white"

                    Row {
                        anchors.fill: parent
                        spacing: 10

                        // Image {
                        //     source: "images/" + model.weatherIcon + ".png"
                        //     width: 50
                        //     height: 50
                        //     anchors.verticalCenter: parent.verticalCenter
                        // }

                        Column {
                            Label {
                                text: model.cityName
                                font.pixelSize: 20
                            }

                            Label {
                                text: model.weatherMain
                                font.pixelSize: 16
                            }

                            Label {
                                text: model.weatherDescription
                                font.pixelSize: 14
                            }
                        }

                        Column {
                            Label {
                                text: "Temp: " + model.mainTemp + "°C"
                                font.pixelSize: 16
                            }

                            Label {
                                text: "Wind: " + model.windSpeed + " m/s"
                                font.pixelSize: 16
                            }

                            Label {
                                text: "Rain: " + model.rain3h + " mm"
                                font.pixelSize: 16
                            }
                        }
                    }
                }
            }
        }

        // Weather forecast
        Rectangle {
            id: forecast
            color: "lightgreen"

            ListView {
                id: listViewForecast
                model: weatherModel
                delegate: Rectangle {
                    visible: !model.isCurrentWeather
                    width: listViewForecast.width
                    height: visible ? 100 : 0
                    color: index % 2 == 0 ? "lightgray" : "white"

                    Row {
                        anchors.fill: parent
                        spacing: 10

                        // Image {
                        //     source: "images/" + model.weatherIcon + ".png"
                        //     width: 50
                        //     height: 50
                        //     anchors.verticalCenter: parent.verticalCenter
                        // }

                        Column {
                            Label {
                                text: model.cityName
                                font.pixelSize: 20
                            }

                            Label {
                                text: model.weatherMain
                                font.pixelSize: 16
                            }

                            Label {
                                text: model.weatherDescription
                                font.pixelSize: 14
                            }
                        }

                        Column {
                            Label {
                                text: "Temp: " + model.mainTemp + "°C"
                                font.pixelSize: 16
                            }

                            Label {
                                text: "Wind: " + model.windSpeed + " m/s"
                                font.pixelSize: 16
                            }

                            Label {
                                text: "Rain: " + model.rain3h + " mm"
                                font.pixelSize: 16
                            }
                        }
                    }
                }
            }
        }
    }
}
