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

    QString weatherDescription() const;

    QString weatherMain() const;

signals:

private:
    QString m_city;
    QString m_weatherDescription;
    QString m_weatherMain;
};

#endif // WEATHERDATA_H
