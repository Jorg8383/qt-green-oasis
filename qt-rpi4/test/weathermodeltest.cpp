#include "weathermodeltest.h"

WeatherModelTest::WeatherModelTest(QObject *parent)
    : QObject{parent}
{

}

WeatherModelTest::~WeatherModelTest()
{
    qDeleteAll(m_weatherDataList); // Call 'delete' on all items in the list
}

// This method will be invoked by the test framework before the first test function is executed
void WeatherModelTest::initTestCase()
{
    // Get test data from a external JSON file
    QFile file("../test/data/test_data_weather.json");
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
}

// This method will be invoked by the test framework after the last test function was executed
void WeatherModelTest::cleanupTestCase()
{
    qDeleteAll(m_weatherDataList); // Call 'delete' on all items in the list
    m_weatherDataList.clear(); // Remove items from the list
}

void WeatherModelTest::testRowCount()
{

    WeatherModel model;
    model.setWeatherData(m_weatherDataList);

    QCOMPARE(model.rowCount(), m_weatherDataList.count());
}

void WeatherModelTest::testData()
{
    WeatherModel model;
    model.setWeatherData(m_weatherDataList);

    for (int i = 0; i < m_weatherDataList.count(); i++)
    {
        QCOMPARE(model.data(model.index(i), WeatherModel::CityNameRole).toString(), m_weatherDataList[i]->cityName());
        QCOMPARE(model.data(model.index(i), WeatherModel::WeatherDescriptionRole).toString(), m_weatherDataList[i]->weatherDescription());
        QCOMPARE(model.data(model.index(i), WeatherModel::WeatherMainRole).toString(), m_weatherDataList[i]->weatherMain());
        QCOMPARE(model.data(model.index(i), WeatherModel::WeatherIconRole).toString(), m_weatherDataList[i]->weatherIcon());
        QCOMPARE(model.data(model.index(i), WeatherModel::TemperatureRole).toDouble(), m_weatherDataList[i]->mainTemp());
        QCOMPARE(model.data(model.index(i), WeatherModel::MinTemperatureRole).toDouble(), m_weatherDataList[i]->mainTempMin());
        QCOMPARE(model.data(model.index(i), WeatherModel::MaxTemperatureRole).toDouble(), m_weatherDataList[i]->mainTempMax());
        QCOMPARE(model.data(model.index(i), WeatherModel::WindSpeedRole).toDouble(), m_weatherDataList[i]->windSpeed());
        QCOMPARE(model.data(model.index(i), WeatherModel::Rain3hRole).toDouble(), m_weatherDataList[i]->rain3h());
        QCOMPARE(model.data(model.index(i), WeatherModel::Snow3hRole).toDouble(), m_weatherDataList[i]->snow3h());
        QCOMPARE(model.data(model.index(i), WeatherModel::PopRole).toDouble(), m_weatherDataList[i]->pop());
    }
}
