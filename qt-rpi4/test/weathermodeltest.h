#ifndef WEATHERMODELTEST_H
#define WEATHERMODELTEST_H

#include <QObject>
#include <QDebug>
#include <QTest>
#include <QFile>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>
#include <QJsonDocument>
#include <QJsonParseError>
#include <weatherdata.h>
#include <weathermodel.h>

class WeatherModelTest : public QObject
{
    Q_OBJECT
public:
    explicit WeatherModelTest(QObject *parent = nullptr);
    ~WeatherModelTest(); // Deconstructor

signals:

private slots:    
    void initTestCase(); // Will be called before the first test function is executed
    void cleanupTestCase(); // Will be called after the last test function was executed

    void testRowCount();
    void testData();

private:
    QList<WeatherData*> m_weatherDataList;
    WeatherModel m_model;
};

#endif // WEATHERMODELTEST_H
