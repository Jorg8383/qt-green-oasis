import QtQuick
import QtQuick.Controls

// Import C++ modules into QML
// import com.greenoasis.weathermodel
// import com.greenoasis.weatherdata

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
