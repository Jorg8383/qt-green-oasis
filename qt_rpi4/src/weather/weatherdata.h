#ifndef WEATHERDATA_H
#define WEATHERDATA_H

#include <QObject>
#include <QDateTime>
#include <QJsonObject>
#include <QJsonArray>


class WeatherData : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool isCurrent READ isCurrentWeather NOTIFY dataChanged)
    Q_PROPERTY(int dt READ dt NOTIFY dataChanged)
    Q_PROPERTY(QDateTime qdt READ qDateTime NOTIFY dataChanged)
    Q_PROPERTY(QString cityName READ cityName NOTIFY dataChanged)
    Q_PROPERTY(QString weatherId READ weatherId NOTIFY dataChanged)
    Q_PROPERTY(QString weatherMain READ weatherMain NOTIFY dataChanged)
    Q_PROPERTY(QString weatherDescription READ weatherDescription NOTIFY dataChanged)
    Q_PROPERTY(QString weatherIcon READ weatherIcon NOTIFY dataChanged)
    Q_PROPERTY(double mainTemp READ mainTemp NOTIFY dataChanged)
    Q_PROPERTY(double mainTempMin READ mainTempMin NOTIFY dataChanged)
    Q_PROPERTY(double mainTempMax READ mainTempMax NOTIFY dataChanged)
    Q_PROPERTY(double windSpeed READ windSpeed NOTIFY dataChanged)
    Q_PROPERTY(double snow3h READ snow3h NOTIFY dataChanged)
    Q_PROPERTY(double rain3h READ rain3h NOTIFY dataChanged)
    Q_PROPERTY(double pop READ pop NOTIFY dataChanged)

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
    double pop() const;

signals:
    void dataChanged();

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
    double m_pop; // Probability of precipitation [%]

};

#endif // WEATHERDATA_H
