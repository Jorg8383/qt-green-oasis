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
    QString m_city;
    QString m_weatherDescription;
    QString m_weatherMain;
};

#endif // WEATHERDATA_H
