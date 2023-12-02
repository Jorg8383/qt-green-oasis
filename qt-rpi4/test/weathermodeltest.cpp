#include "weathermodeltest.h"

WeatherModelTest::WeatherModelTest(QObject *parent)
    : QObject{parent}
{
    setObjectName("WeatherModelTest");
}

WeatherModelTest::~WeatherModelTest()
{

}

// This method will be invoked by the test framework before the first test function is executed
void WeatherModelTest::initTestCase()
{

    // Get test data from a external JSON file
    QFile file("../../qt-rpi4/test/data/test_data_weather.json");
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << this << "Couldn't open file: " << file.fileName() << " Error: " << file.errorString();
        return;
    }

    // Parse the JSON data
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError)
    {
        qWarning() << this << "Failed to parse JSON: " << parseError.errorString();
        return;
    }

    // Check whether the 'list' array exists
    QJsonObject obj = doc.object();
    if (!obj.contains("list") || !obj["list"].isArray())
    {
        qWarning() << this << "JSON does not contain a 'list' array!";
        return;
    }

    // Get the city name
    QString cityName;
    if (!obj.contains("city") || !obj["city"].isObject())
    {
        qWarning() << this << "JSON does not contain a 'city' object!";
        return;
    }
    else
    {
        QJsonObject cityObject = obj["city"].toObject();
        cityName = cityObject["name"].toString();
    }

    // Create WeatherData objects and populate them with the retrieved data
    QJsonArray list = obj["list"].toArray();
    bool isCurrentWeather = true;
    for (const QJsonValue& item : list)
    {
        // Convert the item to a object and use date & time as the object name
        QJsonObject dataItem = item.toObject();
        QString dataItemName = dataItem["dt_txt"].toString();
        // Create a new weather data object and append it to the weather data list
        WeatherData* weatherData = new WeatherData(dataItemName, dataItem, cityName, isCurrentWeather);
        m_weatherDataList.append(weatherData);
        isCurrentWeather = false;
    }

    // Pass the created list to the model which will take ownership for all WeatherData objects
    m_model.setWeatherData(m_weatherDataList);

}

// This method will be invoked by the test framework after the last test function was executed
void WeatherModelTest::cleanupTestCase()
{
    /*
     * Note: Don't use qDeleteAll(m_weatherDataList) here!!!
     * After invoking weatherModel.setWeatherData(QList<WeatherData*> newData),
     * the model takes ownership of these weather data objects and deletes them
     * when the model gets destyoyed (deconstructed). Here, in this cleanupTestCase()
     * method we only remove these items from the list but we DO NOT delete them.
     * Double deletion of objects can cause unpredicted behaviour and crash the
     * application!
    */
    m_weatherDataList.clear(); // Remove items from the list
}

void WeatherModelTest::testRowCount()
{
    QCOMPARE(m_model.rowCount(), m_weatherDataList.count());
}


void WeatherModelTest::testData()
{
    for (int i = 0; i < m_weatherDataList.count(); i++)
    {
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::WeatherMainRole).toString(), m_weatherDataList[i]->weatherMain());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::WeatherDescriptionRole).toString(), m_weatherDataList[i]->weatherDescription());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::CityNameRole).toString(), m_weatherDataList[i]->cityName());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::WeatherIconRole).toString(), m_weatherDataList[i]->weatherIcon());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::TemperatureRole).toDouble(), m_weatherDataList[i]->mainTemp());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::MinTemperatureRole).toDouble(), m_weatherDataList[i]->mainTempMin());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::MaxTemperatureRole).toDouble(), m_weatherDataList[i]->mainTempMax());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::WindSpeedRole).toDouble(), m_weatherDataList[i]->windSpeed());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::Rain3hRole).toDouble(), m_weatherDataList[i]->rain3h());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::Snow3hRole).toDouble(), m_weatherDataList[i]->snow3h());
        QCOMPARE(m_model.data(m_model.index(i), WeatherModel::PopRole).toDouble(), m_weatherDataList[i]->pop());
    }
}

