#include "weatherdatatest.h"

WeatherDataTest::WeatherDataTest(QObject *parent)
    : QObject{parent}
{

}

void WeatherDataTest::testConstructor()
{


}

void WeatherDataTest::testConstructor_data()
{

    // Prepare JSON test data set #1 - including all properties
    QJsonObject testDataSet1;
    testDataSet1.insert("dt", 1701421200);
    QJsonObject mainObject;
    mainObject.insert("temp", 20.3);
    mainObject.insert("temp_min", 18.8);
    mainObject.insert("temp_max", 23.4);
    testDataSet1.insert("main", mainObject);
    QJsonArray weatherArray;
    QJsonObject weatherObject;
    weatherObject.insert("id", 500);
    weatherObject.insert("main", "Rain");
    weatherObject.insert("description", "light rain");
    weatherObject.insert("icon", "10d");
    weatherArray.append(weatherObject);
    testDataSet1.insert("weather", weatherObject);
    QJsonObject windObject;
    windObject.insert("speed", 1.97);
    testDataSet1.insert("wind", windObject);
    QJsonObject rainObject;
    rainObject.insert("3h", 0.57);
    testDataSet1.insert("rain", rainObject);
    QJsonObject snowObject;
    snowObject.insert("3h", 0.52);
    testDataSet1.insert("snow", snowObject);
    testDataSet1.insert("pop", 0.33);
}

void WeatherDataTest::testProperties()
{

}

void WeatherDataTest::testProperties_data()
{

}

void WeatherDataTest::initTestCase()
{

}

void WeatherDataTest::initTestCase_data()
{
    // Create a global test table
    QTest::addColumn<QJsonObject>("data");
    QTest::addColumn<bool>("isCurrentWeather");
    QTest::addColumn<int>("dt");
    QTest::addColumn<QDateTime>("qDateTime");
    QTest::addColumn<QString>("cityName");
    QTest::addColumn<QString>("weatherId");
    QTest::addColumn<QString>("weatherMain");
    QTest::addColumn<QString>("weatherDescription");
    QTest::addColumn<QString>("weatherIcon");
    QTest::addColumn<double>("mainTemp");
    QTest::addColumn<double>("mainTempMin");
    QTest::addColumn<double>("mainTempMax");
    QTest::addColumn<double>("windSpeed");
    QTest::addColumn<double>("snow3h");
    QTest::addColumn<double>("rain3h");
    QTest::addColumn<int>("pop");
}

void WeatherDataTest::cleanupTestCase()
{

}

void WeatherDataTest::init()
{

}

void WeatherDataTest::cleanup()
{

}
