#include "weatherdata.h"

WeatherData::WeatherData(QObject *parent)
    : QObject{parent}
{

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
