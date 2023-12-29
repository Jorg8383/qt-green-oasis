#include "weatherfetchertest.h"

WeatherFetcherTest::WeatherFetcherTest(QObject *parent)
    : QObject{parent}
{}

void WeatherFetcherTest::testWeatherRequest()
{
    // Create a mock network reply
    std::unique_ptr<MockNetworkReply> mockReply(new MockNetworkReply(m_jsonData));

    // Crate a mock network manager

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
