#include "weatherdatatest.h"

WeatherDataTest::WeatherDataTest(QObject *parent)
    : QObject{parent}
{

}

void WeatherDataTest::testSetAndGet()
{
    WeatherData data;
    data.setCity("Test City");
    QCOMPARE(data.city(), QString("Test City"));

    data.setWeatherDescription("Test Description");
    QCOMPARE(data.weatherDescription(), QString("Test Description"));

    data.setWeatherMain("Test Main");
    QCOMPARE(data.weatherMain(), QString("Test Main"));
}
