#include "weatherdata.h"

WeatherData::WeatherData(QString objectName,
                         const QJsonObject &data,
                         const QString &cityName,
                         const bool isCurrentWeather,
                         QObject *parent)
    : QObject{parent}
{
    setObjectName(objectName);
    m_city = cityName;
    m_isCurrentWeather = isCurrentWeather;

    if (!data.isEmpty())
    {
        extractData(data);
    }
    else
    {
        qWarning() << this << " QJsonObject of weather data is empty!";
    }

}

void WeatherData::extractData(const QJsonObject &data)
{
    // Extract time of data forecast as UNIX timestamp in seconds
    if (data.contains("dt"))
    {
        m_dt = data["dt"].toInt();
    }

    // Extract "main" properties
    if (data.contains("main"))
    {
        QJsonObject mainObject = data["main"].toObject();
        if (mainObject.contains("temp"))
        {
            m_temperature = mainObject["temp"].toDouble();
        }
        if (mainObject.contains("temp_min"))
        {
            m_minTemperature = mainObject["temp_min"].toDouble();
        }
        if (mainObject.contains("temp_max"))
        {
            m_maxTemperature = mainObject["temp_max"].toDouble();
        }
        if (mainObject.contains("humidity"))
        {
            m_humidity = mainObject["humidity"].toInt();
        }
    }

    // Extract "weather" properties
    if (data.contains("weather"))
    {
        QJsonArray weatherArray = data["weather"].toArray();
        QJsonObject weatherObject = weatherArray.at(0).toObject();
        if (weatherObject.contains("main"))
        {
            m_weatherMain = weatherObject["main"].toString();
        }
        if (weatherObject.contains("description"))
        {
            m_weatherDescription = weatherObject["description"].toString();
        }
        if (weatherObject.contains("icon"))
        {
            m_weatherIcon = weatherObject["icon"].toString();
        }
    }

    // Extract "clouds" properties
    if (data.contains("clouds"))
    {
        QJsonObject cloudsObject = data["clouds"].toObject();
        if (cloudsObject.contains("all"))
        {
            m_cloudiness = cloudsObject["all"].toInt();
        }
    }

    // Extract "wind" properties
    if (data.contains("wind"))
    {
        QJsonObject windObject = data["wind"].toObject();
        if (windObject.contains("speed"))
        {
            m_windSpeed = windObject["speed"].toDouble();
        }
    }

    // Extract "pop" properties
    if (data.contains("pop"))
    {
        m_pop = data["pop"].toInt();
    }
}

bool WeatherData::isCurrentWeather() const
{
    return m_isCurrentWeather;
}

int WeatherData::getDt() const
{
    return m_dt;
}

QDateTime WeatherData::qDateTime() const
{
    return m_qDateTime;
}

QString WeatherData::city() const
{
    return m_city;
}

QString WeatherData::weatherMain() const
{
    return m_weatherMain;
}

QString WeatherData::weatherDescription() const
{
    return m_weatherDescription;
}

QString WeatherData::weatherIcon() const
{
    return m_weatherIcon;
}

double WeatherData::temperature() const
{
    return m_temperature;
}

double WeatherData::minTemperature() const
{
    return m_minTemperature;
}

double WeatherData::maxTemperature() const
{
    return m_maxTemperature;
}

double WeatherData::windSpeed() const
{
    return m_windSpeed;
}

double WeatherData::snow() const
{
    return m_snow;
}

int WeatherData::humidity() const
{
    return m_humidity;
}

int WeatherData::cloudiness() const
{
    return m_cloudiness;
}

QString WeatherData::weatherId() const
{
    return m_weatherId;
}



