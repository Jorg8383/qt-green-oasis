#ifndef TESTWEATHERDATA_H
#define TESTWEATHERDATA_H

#include <QObject>

class TestWeatherData : public QObject
{
    Q_OBJECT
public:
    explicit TestWeatherData(QObject *parent = nullptr);

signals:

private slots:
    void testSetWeatherData();

};

#endif // TESTWEATHERDATA_H
