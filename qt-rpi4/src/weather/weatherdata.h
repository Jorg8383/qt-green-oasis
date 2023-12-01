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
    int dt() const;
    QDateTime qDateTime() const;
    QString cityName() const;
    QString weatherId() const;
    QString weatherMain() const;
    QString weatherDescription() const;
    QString weatherIcon() const;
    double mainTemp() const;
    double mainTempMin() const;
    double mainTempMax() const;
    double windSpeed() const;
    double snow3h() const;
    double rain3h() const;
    int pop() const;

signals:

private:
    void extractData(const QJsonObject &data);

    bool m_isCurrentWeather; // Flag to differentiate between current weather and forecast item
    int m_dt; // Unix timestamp in seconds
    QDateTime m_qDateTime; // Time of data forecasted, unix, UTC
    QString m_cityName; // City name
    QString m_weatherId; // Weahter ID
    QString m_weatherMain; // e.g. "Clouds"
    QString m_weatherDescription; // e.g. "overcast clouds"
    QString m_weatherIcon; // Weather icon id
    double m_mainTemp; // Temperature
    double m_mainTempMin; // Min. Temperature
    double m_mainTempMax; // Max. Temperature
    double m_windSpeed; // Wind speed [m/s]
    double m_snow3h; // Snow volume for the last 3 hours [mm]
    double m_rain3h; // Rain volume for the last 3 hours [mm]
    int m_pop; // Probability of precipitation [%]

};

#endif // WEATHERDATA_H
