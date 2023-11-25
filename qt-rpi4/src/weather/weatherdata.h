#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QObject>

class WeatherData : public QObject
{
    Q_OBJECT
public:
    explicit WeatherData(QObject *parent = nullptr);

    void setWeatherData(const QString& city, const QString& description, const QString& main);



    QString city() const;
    void setCity(const QString &newCity);

    QString weatherDescription() const;
    void setWeatherDescription(const QString &newWeatherDescription);

    QString weatherMain() const;
    void setWeatherMain(const QString &newWeatherMain);

signals:

private:
    bool m_isCurrentWeather; // Flag to differentiate between current weather and forecast item

    int dt; // Unix timestamp in seconds
    QDateTime m_dateTime; // Date and time format
    QString m_city; // City name

    QString m_weatherMain; // e.g. "Clouds"
    QString m_weatherDescription; // e.g. "overcast clouds"
    QString m_weatherIcon; // Weather icon id

    double m_temperature; // Temperature
    double m_minTemperature; // Min. Temperature
    double m_maxTemperature; // Max. Temperature
    int m_humidity; // Humidity [%]
    int m_cloudiness; // Cloudiness [%]
    double m_windSpeed; // Wind speed [m/s]

    double m_snow; // Snow volume [mm]
};

#endif // WEATHERDATA_H
