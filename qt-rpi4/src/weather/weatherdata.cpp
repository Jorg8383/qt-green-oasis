#include "weatherdata.h"

WeatherData::WeatherData(QString objectName,
                         const QJsonObject &data,
                         const QString &cityName,
                         const bool isCurrentWeather,
                         QObject *parent)
    : QObject{parent}
{
    setObjectName(objectName);
    m_cityName = cityName;
    m_isCurrentWeather = isCurrentWeather;

    if (!data.isEmpty())
    {
        extractData(data);
    }
    else
    {
        qWarning() << this << "Weather data - QJsonObject - is empty!";
    }

}

void WeatherData::extractData(const QJsonObject &data)
{
    // Extract time of data forecast as UNIX timestamp in seconds
    if (data.contains("dt"))
    {
        m_dt = data["dt"].toInt();
        m_qDateTime = QDateTime::fromSecsSinceEpoch(m_dt);
    }

    // Extract "main" properties
    if (data.contains("main"))
    {
        QJsonObject mainObject = data["main"].toObject();
        m_mainTemp = mainObject["temp"].toDouble();
        m_mainTempMin = mainObject["temp_min"].toDouble();
        m_mainTempMax = mainObject["temp_max"].toDouble();

//        qDebug() << this << "Write m_mainTempMax: " << m_mainTempMax;
    }

    // Extract "weather" properties
    if (data.contains("weather"))
    {
        QJsonArray weatherArray = data["weather"].toArray();
        QJsonObject weatherObject = weatherArray.at(0).toObject();
        m_weatherId = weatherObject["id"].toString();
        m_weatherMain = weatherObject["main"].toString();
        m_weatherDescription = weatherObject["description"].toString();
        m_weatherIcon = weatherObject["icon"].toString();
    }

    // Extract "wind" properties
    if (data.contains("wind"))
    {
        QJsonObject windObject = data["wind"].toObject();
        m_windSpeed = windObject["speed"].toDouble();
    }

    // Extract "pop" properties
    if (data.contains("pop"))
    {
        m_pop = data["pop"].toDouble();
    }

    // Extract "rain" properties
    if (data.contains("rain"))
    {
        QJsonObject rainObject = data["rain"].toObject();
        m_rain3h = rainObject["3h"].toDouble();
    }

    // Extract "snow" properties
    if (data.contains("snow"))
    {
        QJsonObject snowObject = data["snow"].toObject();
        m_snow3h = snowObject["3h"].toDouble();
    }
}

double WeatherData::pop() const
{
    return m_pop;
}

double WeatherData::rain3h() const
{
    return m_rain3h;
}

double WeatherData::snow3h() const
{
    return m_snow3h;
}

double WeatherData::windSpeed() const
{
    return m_windSpeed;
}

double WeatherData::mainTempMax() const
{
    return m_mainTempMax;
}

double WeatherData::mainTempMin() const
{
    return m_mainTempMin;
}

double WeatherData::mainTemp() const
{
    return m_mainTemp;
}

QString WeatherData::weatherIcon() const
{
    return m_weatherIcon;
}

QString WeatherData::weatherDescription() const
{
    return m_weatherDescription;
}

QString WeatherData::weatherMain() const
{
    return m_weatherMain;
}

QString WeatherData::weatherId() const
{
    return m_weatherId;
}

QString WeatherData::cityName() const
{
    return m_cityName;
}

QDateTime WeatherData::qDateTime() const
{
    return m_qDateTime;
}

int WeatherData::dt() const
{
    return m_dt;
}

bool WeatherData::isCurrentWeather() const
{
    return m_isCurrentWeather;
}




