import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.greenoasis.weather 1.0

ApplicationWindow {
    visible: true
    width: 640
    height: 480
    title: qsTr("Weather Model ListView Test")

    // Assuming 'weatherModel' is the id of your WeatherModel instance
    ListView {
        id: listView
        anchors.fill: parent
        model: weatherModel // Set your WeatherModel instance as the model

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


/*
ApplicationWindow {
    id: window
    visible: true
    width: 800
    height: 480
    title: qsTr("Green-Oasis")

    SwipeView {
        id: swipeView
        anchors.fill: parent
        currentIndex: 0

        WeatherPage{
            id: weatherPage
        }

        // Add more pages here...

    }

    PageIndicator {
        id: pageIndicator
        count: swipeView.count
        currentIndex: swipeView.currentIndex
        anchors.bottom: swipeView.bottom
        anchors.horizontalCenter: swipeView.horizontalCenter
    }

}
*/
