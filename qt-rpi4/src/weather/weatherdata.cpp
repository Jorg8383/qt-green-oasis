#include "weatherdata.h"

WeatherData::WeatherData(QObject *parent)
    : QObject{parent}
{

}

void WeatherData::setWeatherData(const QString &city, const QString &description, const QString &main)
{
    m_city = city;
    m_weatherDescription = description;
    m_weatherMain = main;
}

QString WeatherData::city() const
{
    return m_city;
}

QString WeatherData::weatherDescription() const
{
    return m_weatherDescription;
}

QString WeatherData::weatherMain() const
{
    return m_weatherMain;
}
