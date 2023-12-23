import QtQuick 2.15

import com.greenoasis.weathermodel
import com.greenoasis.weatherdata

Item {
    ListView {
        id: forecastList
        anchors.right: parent.right
        width: parent.width / 2
        height: parent.height
        model: WeatherModel
        delegate: Item {
            width: forecastList.width
            height: 100
            Row {
                Image {
                    source: "images/" + model.weatherIcon + ".png"
                }
                Column {
                    Text { text: "Temp: " + model.mainTemp }
                    Text { text: "Min Temp: " + model.mainTempMin }
                    Text { text: "Max Temp: " + model.mainTempMax }
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
            source: "images/" + WeatherModel.get(0).weatherIcon + ".png"
        }
        Text { text: "Current Temp: " + WeatherModel.get(0).mainTemp }
        Text { text: "Min Temp: " + WeatherModel.get(0).mainTempMin }
        Text { text: "Max Temp: " + WeatherModel.get(0).mainTempMax }
    }
}
