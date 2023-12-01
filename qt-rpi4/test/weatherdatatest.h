#ifndef WEATHERDATATEST_H
#define WEATHERDATATEST_H

#include <QObject>
#include <QTest>
#include <QJsonObject>
#include <QDateTime>
#include <QJsonArray>
#include <weatherdata.h>

class WeatherDataTest : public QObject
{
    Q_OBJECT
public:
    explicit WeatherDataTest(QObject *parent = nullptr);

signals:

private slots:
    // Define test functions
    void testConstructorWithAllData();
    void testConstructorWithAllData_data();
    void testConstructorWithoutSnowData();
    void testConstructorWithoutSnowData_data();
    void testProperties();
    void testProperties_data();

    // Define methodes that are automatically invoked by the test framework
    void initTestCase(); // Will be called before the first test function is executed
    void initTestCase_data(); // Will be called to create a global test data table
    void cleanupTestCase(); // Will be called after the last test function was executed
    void init(); // Will be called after the last test function was executed
    void cleanup(); // Will be called after the last test function was executed

private:
    QDateTime m_qDateTime;
    const quint64 m_timestampMillis = 1701421200;

};

#endif // WEATHERDATATEST_H
