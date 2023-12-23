import QtQuick 2.15

// Item {
//     id: weatherpage
//     anchors.verticalCenter: parent.verticalCenter
//     anchors.horizontalCenter: parent.horizontalCenter

//     Rectangle {

//         Text {
//             id: name
//             text: qsTr("Green Oasis Pi")
//         }
//     }

// }

//Import C++ modules
// import com.weather.weatherdata
// import com.weather.weathermodel

Item {
    ListView {
        id: forecastList
        anchors.right: parent.right
        width: parent.width / 2
        height: parent.height
        model: weatherModel
        delegate: Item {
            width: forecastList.width
            height: 100
            Row {
                Image {
                    source: "images/" + weatherIcon + ".png"
                }
                Column {
                    Text { text: "Temp: " + mainTemp }
                    Text { text: "Min Temp: " + mainTempMin }
                    Text { text: "Max Temp: " + mainTempMax }
                }
            }
        }
    }

    Column {
        anchors.left: parent.left
        width: parent.width / 2
        height: parent.height
        Image {
            id: currentWeatherIcon
            source: "images/" + weatherModel.get(0).weatherIcon + ".png"
        }
        Text { text: "Current Temp: " + weatherModel.get(0).mainTemp }
        Text { text: "Min Temp: " + weatherModel.get(0).mainTempMin }
        Text { text: "Max Temp: " + weatherModel.get(0).mainTempMax }
    }
}
