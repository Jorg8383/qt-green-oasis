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

    // Assuming 'weatherModel' is the id of your WeatherModel instance
    ListView {
        id: listView
        anchors.fill: parent
        model: propModel // Set your WeatherModel instance as the model

        delegate: Item {
            width: listView.width
            height: 100 // Set a fixed height for each list item

            RowLayout {
                anchors.fill: parent
                spacing: 10

                // Display weather data using the role names defined in the WeatherModel
                Label {
                    text: cityName
                    font.pixelSize: 20
                }
                Label {
                    text: weatherDescription
                    font.pixelSize: 16
                }
                Label {
                    text: "Temp: " + mainTemp + "°C"
                    font.pixelSize: 16
                }
                Label {
                    text: "Wind: " + windSpeed + " m/s"
                    font.pixelSize: 16
                }
                // ... add more Labels for additional data as needed
            }
        }
    }
}
