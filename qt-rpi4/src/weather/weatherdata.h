#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>


class WeatherData : public QObject
{
    Q_OBJECT
public:
    explicit WeatherData(QString objectName = "WeatherData",
                         const QJsonObject& data = QJsonObject(),
                         const QString& cityName = "",
                         const bool isCurrentWeather = false,
                         QObject *parent = nullptr);

    // Properties
    bool isCurrentWeather() const;
    int getDt() const;
    QDateTime qDateTime() const;
    QString city() const;
    QString weatherMain() const;
    QString weatherDescription() const;
    QString weatherIcon() const;
    double temperature() const;
    double minTemperature() const;
    double maxTemperature() const;
    double windSpeed() const;
    double snow() const;
    int humidity() const;
    int cloudiness() const;

    QString weatherId() const;

signals:

private:
    void extractData(const QJsonObject &data);

    bool m_isCurrentWeather; // Flag to differentiate between current weather and forecast item
    int m_dt; // Unix timestamp in seconds
    QDateTime m_qDateTime; // Date and time format
    QString m_city; // City name
    QString m_weatherId; // Weahter ID
    QString m_weatherMain; // e.g. "Clouds"
    QString m_weatherDescription; // e.g. "overcast clouds"
    QString m_weatherIcon; // Weather icon id
    double m_temperature; // Temperature
    double m_minTemperature; // Min. Temperature
    double m_maxTemperature; // Max. Temperature
    double m_windSpeed; // Wind speed [m/s]
    double m_snow; // Snow volume [mm]
    int m_humidity; // Humidity [%]
    int m_cloudiness; // Cloudiness [%]
    int m_pop; // Probability of precipitation [%]

};

#endif // WEATHERDATA_H
