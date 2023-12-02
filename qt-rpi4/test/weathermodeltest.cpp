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
//    WeatherModel model;
//    QList<WeatherData*> weatherDataList;

//    // Create a new weather data object
//    WeatherData* data = new WeatherData();
//    data->setCity("Dublin");
//    data->setMaxTemperature(6.5);
//    data->setMinTemperature(-3.5);
//    weatherDataList.append(data);

//    // Create another weather data object
//    data = new WeatherData();
//    data->setCity("Dublin");
//    data->setMaxTemperature(3.5);
//    data->setMinTemperature(-2.5);
//    weatherDataList.append(data);

//    // Pass the weather data list to the model
//    model.setWeatherData(weatherDataList);

//    QCOMPARE(model.data(model.index(0), WeatherModel::CityRole), QString("Dublin"));
//    QCOMPARE(model.data(model.index(0), WeatherModel::MaxTemperatureRole), 6.5);
//    QCOMPARE(model.data(model.index(0), WeatherModel::MinTemperatureRole), -3.5);
//    QCOMPARE(model.data(model.index(1), WeatherModel::CityRole), QString("Dublin"));
//    QCOMPARE(model.data(model.index(1), WeatherModel::MaxTemperatureRole), 3.5);
//    QCOMPARE(model.data(model.index(1), WeatherModel::MinTemperatureRole), -2.5);

}
