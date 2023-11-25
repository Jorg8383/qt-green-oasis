#ifndef WEATHERDATATEST_H
#define WEATHERDATATEST_H

#include <QObject>
#include <weatherdata.h>

class WeatherDataTest : public QObject
{
    Q_OBJECT
public:
    explicit WeatherDataTest(QObject *parent = nullptr);

signals:

private slots:
    void testSetWeatherData();

};

#endif // WEATHERDATATEST_H
