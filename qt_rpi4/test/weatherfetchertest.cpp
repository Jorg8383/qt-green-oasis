#include "weatherfetchertest.h"
#include "mocknetworkreply.h"
#include "mocknetworkaccessmanager.h"
#include "weathermodel.h"
#include "weatherfetcher.h"
#include "configmanager.h"

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
