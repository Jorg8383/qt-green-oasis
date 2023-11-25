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

void WeatherData::setCity(const QString &newCity)
{
    m_city = newCity;
}

QString WeatherData::weatherDescription() const
{
    return m_weatherDescription;
}

void WeatherData::setWeatherDescription(const QString &newWeatherDescription)
{
    m_weatherDescription = newWeatherDescription;
}

QString WeatherData::weatherMain() const
{
    return m_weatherMain;
}

void WeatherData::setWeatherMain(const QString &newWeatherMain)
{
    m_weatherMain = newWeatherMain;
}

