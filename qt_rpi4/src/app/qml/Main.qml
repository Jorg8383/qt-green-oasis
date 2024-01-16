import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

import com.greenoasis.weather 1.0


Window {
    visible: true
    width: 800
    height: 480
    title: qsTr("Green Oasis")

    WeatherPage {
        id: weatherPage
        propModel: weatherModel
    }
}
