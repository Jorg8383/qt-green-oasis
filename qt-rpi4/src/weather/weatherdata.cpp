#include "weatherdata.h"

WeatherData::WeatherData(QString objectName, QObject *parent)
    : QObject{parent}
{
    setObjectName(objectName);
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

bool WeatherData::isCurrentWeather() const
{
    return m_isCurrentWeather;
}

void WeatherData::setIsCurrentWeather(bool newIsCurrentWeather)
{
    m_isCurrentWeather = newIsCurrentWeather;
}

int WeatherData::getDt() const
{
    return dt;
}

void WeatherData::setDt(int newDt)
{
    dt = newDt;
    // Todo: convert dt into QDateTime and store it in m_qDateTime
}

QDateTime WeatherData::qDateTime() const
{
    return m_qDateTime;
}

QString WeatherData::weatherIcon() const
{
    return m_weatherIcon;
}

void WeatherData::setWeatherIcon(const QString &newWeatherIcon)
{
    m_weatherIcon = newWeatherIcon;
}

double WeatherData::temperature() const
{
    return m_temperature;
}

void WeatherData::setTemperature(double newTemperature)
{
    m_temperature = newTemperature;
}

double WeatherData::minTemperature() const
{
    return m_minTemperature;
}

void WeatherData::setMinTemperature(double newMinTemperature)
{
    m_minTemperature = newMinTemperature;
}

double WeatherData::maxTemperature() const
{
    return m_maxTemperature;
}

void WeatherData::setMaxTemperature(double newMaxTemperature)
{
    m_maxTemperature = newMaxTemperature;
}

int WeatherData::humidity() const
{
    return m_humidity;
}

void WeatherData::setHumidity(int newHumidity)
{
    m_humidity = newHumidity;
}

int WeatherData::cloudiness() const
{
    return m_cloudiness;
}

void WeatherData::setCloudiness(int newCloudiness)
{
    m_cloudiness = newCloudiness;
}

double WeatherData::windSpeed() const
{
    return m_windSpeed;
}

void WeatherData::setWindSpeed(double newWindSpeed)
{
    m_windSpeed = newWindSpeed;
}

double WeatherData::snow() const
{
    return m_snow;
}

void WeatherData::setSnow(double newSnow)
{
    m_snow = newSnow;
}

