#include "weatherfetchertest.h"
#include "mocknetworkreply.h"
#include "mocknetworkaccessmanager.h"
#include <weathermodel.h>
#include <weatherfetcher.h>
#include <configmanager.h>

WeatherFetcherTest::WeatherFetcherTest(QObject *parent)
    : QObject{parent}
{}

void WeatherFetcherTest::testWeatherRequest()
{
    // Create a mock network manager
    MockNetworkAccessManager networkManager(m_jsonData);

    // Create a WeatherModel
    WeatherModel weatherModel;

    // Get the API key from the config.ini file
    auto apiKey = ConfigManager::instance().getValue("Weather/OpenWeatherApiKey");

    // Create a WeatherFetcher instance for testing
    WeatherFetcher weatherFetcher(networkManager, weatherModel, apiKey.toString());

    // Connect signals for testing
    QSignalSpy dataUpdatedSpy(&weatherFetcher, &WeatherFetcher::dataUpdated);
    QSignalSpy networkErrorSpy(&weatherFetcher, &WeatherFetcher::networkError);

    // Create an event loop, which is required to utilise QSignalSpy since we're outside of 'main' event loop
    QEventLoop loop;
    QObject::connect(&weatherFetcher, &WeatherFetcher::dataUpdated, &loop, &QEventLoop::quit);

    // Request weather data
    auto latitude = ConfigManager::instance().getValue("Weather/Latitude");
    auto longitude = ConfigManager::instance().getValue("Weather/Longitude");
    weatherFetcher.requestData(latitude.toDouble(), longitude.toDouble());

    // Start the event loop
    loop.exec();

    // Wait for signals to be emitted
    QVERIFY2(dataUpdatedSpy.wait(), "dataUpdated signal not emitted");
    QVERIFY2(networkErrorSpy.isEmpty(), "Unexpected networkError signal");

    // Validate the results
    QCOMPARE(weatherModel.rowCount(), 40);
    QCOMPARE(weatherModel.data(weatherModel.index(0), WeatherModel::WeatherMainRole).toString(), "Clouds");
}

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
    ConfigManager::instance().initialise("../resources/config/config.ini");

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
