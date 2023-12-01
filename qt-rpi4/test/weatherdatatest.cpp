#include "weatherdatatest.h"

WeatherDataTest::WeatherDataTest(QObject *parent)
    : QObject{parent}
{

}

void WeatherDataTest::testConstructorWithAllData()
{
    // Fetch the elements of the data set
    QFETCH(QJsonObject, data);
    QFETCH(bool, isCurrentWeather);
    QFETCH(int, dt);
    QFETCH(QDateTime, qDateTime);
    QFETCH(QString, cityName);
    QFETCH(QString, weatherId);
    QFETCH(QString, weatherMain);
    QFETCH(QString, weatherDescription);
    QFETCH(QString, weatherIcon);
    QFETCH(double, mainTemp);
    QFETCH(double, mainTempMin);
    QFETCH(double, mainTempMax);
    QFETCH(double, windSpeed);
    QFETCH(double, snow3h);
    QFETCH(double, rain3h);
    QFETCH(double, pop);

    // Create a WeatherData object
    WeatherData weatherData("TestWeatherAllData", data, cityName, isCurrentWeather);

    // Verify the data extraction in the constructor
    QCOMPARE(weatherData.dt(), dt);
    QCOMPARE(weatherData.isCurrentWeather(), isCurrentWeather);
    QCOMPARE(weatherData.qDateTime(), qDateTime);
    QCOMPARE(weatherData.cityName(), cityName);
    QCOMPARE(weatherData.weatherId(), weatherId);
    QCOMPARE(weatherData.weatherMain(), weatherMain);
    QCOMPARE(weatherData.weatherDescription(), weatherDescription);
    QCOMPARE(weatherData.weatherIcon(), weatherIcon);
    QCOMPARE(weatherData.mainTemp(), mainTemp);
    QCOMPARE(weatherData.mainTempMin(), mainTempMin);
    QCOMPARE(weatherData.mainTempMax(), mainTempMax);
    QCOMPARE(weatherData.windSpeed(), windSpeed);
    QCOMPARE(weatherData.snow3h(), snow3h);
    QCOMPARE(weatherData.rain3h(), rain3h);
    QCOMPARE(weatherData.pop(), pop);
}

void WeatherDataTest::testConstructorWithAllData_data()
{
    // Create a test table for this test case
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
    QTest::addColumn<double>("pop");

    // Prepare the JSON test data for this test case - including all data
    QJsonObject testData;
    testData.insert("dt", 1701421200);
    QJsonObject mainObject;
    mainObject.insert("temp", 20.3);
    mainObject.insert("temp_min", 18.8);
    mainObject.insert("temp_max", 23.4);
    testData.insert("main", mainObject);
    QJsonArray weatherArray;
    QJsonObject weatherObject;
    weatherObject.insert("id", "500");
    weatherObject.insert("main", "Rain");
    weatherObject.insert("description", "light rain");
    weatherObject.insert("icon", "10d");
    weatherArray.append(weatherObject);
    testData.insert("weather", weatherArray);
    QJsonObject windObject;
    windObject.insert("speed", 1.97);
    testData.insert("wind", windObject);
    QJsonObject rainObject;
    rainObject.insert("3h", 0.52);
    testData.insert("rain", rainObject);
    QJsonObject snowObject;
    snowObject.insert("3h", 0.57);
    testData.insert("snow", snowObject);
    testData.insert("pop", 0.33);

    qInfo() << testData;

    // Add test case with: isCurrentWeather == true
    QTest::newRow("Test case #1") << testData << true << 1701421200 << m_qDateTime
                                      << "London" << "500" << "Rain" << "light rain" << "10d"
                                      << 20.3 << 18.8 << 23.4 << 1.97 << 0.57 << 0.52 << 0.33;
    // Add test case with: isCurrentWeather == false
    QTest::newRow("Test case #2") << testData << false << 1701421200 << m_qDateTime
                                      << "London" << "500" << "Rain" << "light rain" << "10d"
                                      << 20.3 << 18.8 << 23.4 << 1.97 << 0.57 << 0.52 << 0.33;

}

void WeatherDataTest::testConstructorWithoutSnowData()
{

}

void WeatherDataTest::testConstructorWithoutSnowData_data()
{

}

void WeatherDataTest::testProperties()
{

}

void WeatherDataTest::testProperties_data()
{

}

void WeatherDataTest::initTestCase()
{
    // Convert UTC timestamp to QDateTime format
    m_qDateTime = QDateTime::fromSecsSinceEpoch(m_timestampMillis);
}

void WeatherDataTest::initTestCase_data()
{

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
