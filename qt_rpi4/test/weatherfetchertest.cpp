#include "weatherfetchertest.h"
#include <weathermodel.h>
#include <weatherfetcher.h>
#include <configmanager.h>
#include <MockNetworkAccessManager.hpp>


WeatherFetcherTest::WeatherFetcherTest(QObject *parent)
    : QObject{parent}
{}

void WeatherFetcherTest::testWeatherRequest()
{
    // Create and configure the mock network access manager
    MockNetworkAccess::Manager<QNetworkAccessManager> mockNam;

    // Create a WeatherModel
    WeatherModel weatherModel;

    // Get the API key from the config.ini file
    auto apiKey = ConfigManager::instance().getValue("Weather/OpenWeatherApiKey");
    qDebug() << "api key:" << apiKey.toString();

    // Create a WeatherFetcher instance for testing
    WeatherFetcher weatherFetcher(mockNam, weatherModel, apiKey.toString());

    // Configure a fixed JSON response for a matching URL request
    mockNam.whenGet(weatherFetcher.apiUrl()).reply().withBody(m_jsonData);

    // Connect signals for testing
    QSignalSpy dataUpdatedSpy(&weatherFetcher, &WeatherFetcher::dataUpdated);
    QSignalSpy networkErrorSpy(&weatherFetcher, &WeatherFetcher::networkError);


    // Call the method under test by requesting weather data
    auto latitude = ConfigManager::instance().getValue("Weather/Latitude");
    auto longitude = ConfigManager::instance().getValue("Weather/Longitude");
    weatherFetcher.requestData(latitude.toDouble(), longitude.toDouble());

    // Wait for signals to be emitted
    // QVERIFY2(networkFinishedSpy.wait(), "MockNetworkAccessManager::finished signal not emitted");
    QVERIFY2(dataUpdatedSpy.wait(), "dataUpdated signal not emitted");
    QVERIFY2(networkErrorSpy.isEmpty(), "Unexpected networkError signal");

    // Validate the results
    QCOMPARE(weatherModel.rowCount(), 40);
    QCOMPARE(weatherModel.data(weatherModel.index(0), WeatherModel::WeatherMainRole).toString(), "Clouds");

}

// void WeatherFetcherTest::testWeatherRequest()
// {
//     // Create a mock network manager
//     MockNetworkAccessManager networkManager(m_jsonData);

//     // Create a WeatherModel
//     WeatherModel weatherModel;

//     // Get the API key from the config.ini file
//     auto apiKey = ConfigManager::instance().getValue("Weather/OpenWeatherApiKey");

//     // Create a WeatherFetcher instance for testing
//     WeatherFetcher weatherFetcher(networkManager, weatherModel, apiKey.toString());

//     // Connect signals for testing
//     QSignalSpy dataUpdatedSpy(&weatherFetcher, &WeatherFetcher::dataUpdated);
//     QSignalSpy networkErrorSpy(&weatherFetcher, &WeatherFetcher::networkError);
//     QSignalSpy networkFinishedSpy(&networkManager, &MockNetworkAccessManager::finished);

//     // Request weather data
//     auto latitude = ConfigManager::instance().getValue("Weather/Latitude");
//     auto longitude = ConfigManager::instance().getValue("Weather/Longitude");
//     weatherFetcher.requestData(latitude.toDouble(), longitude.toDouble());

//     // Wait for signals to be emitted
//     QVERIFY2(networkFinishedSpy.wait(), "MockNetworkAccessManager::finished signal not emitted");
//     QVERIFY2(dataUpdatedSpy.wait(), "dataUpdated signal not emitted");
//     QVERIFY2(networkErrorSpy.isEmpty(), "Unexpected networkError signal");

//     // Validate the results
//     QCOMPARE(weatherModel.rowCount(), 40);
//     QCOMPARE(weatherModel.data(weatherModel.index(0), WeatherModel::WeatherMainRole).toString(), "Clouds");
// }

void WeatherFetcherTest::testNetworkError()
{

}

void WeatherFetcherTest::initTestCase()
{
    // Get test data from a external JSON file
    QFile file("../../qt_rpi4/test/data/test_data_weather.json");
    if (!file.open(QIODevice::ReadOnly))
    {
        qWarning() << this << "Couldn't open file: " << file.fileName() << " Error: " << file.errorString();
        return;
    }
    m_jsonData = file.readAll();

    // Initialise the ConfigManager
    try {
        ConfigManager::instance().initialise("/home/parallels/QtProjects/qt-green-oasis/qt_rpi4/resources/config/config.ini");
    } catch (const std::exception &e) {
        qDebug() << "Error: " << this << e.what();
    }

}

void WeatherFetcherTest::cleanupTestCase()
{

}

void WeatherFetcherTest::init()
{

}

void WeatherFetcherTest::cleanup()
{

}
