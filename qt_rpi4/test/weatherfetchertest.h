#ifndef WEATHERFETCHERTEST_H
#define WEATHERFETCHERTEST_H

#include <QObject>

class WeatherFetcherTest : public QObject
{
    Q_OBJECT
public:
    explicit WeatherFetcherTest(QObject *parent = nullptr);

signals:

private slots:
    // Define own test functions
    void testWeatherRequest();
    void testNetworkError();

    // Define methodes that are automatically invoked by the test framework
    void initTestCase(); // Will be called before the first test function is executed
    void cleanupTestCase(); // Will be called after the last test function was executed
    void init(); // Will be called before the test function is executed
    void cleanup(); // Will be called after the last test function was executed
};

#endif // WEATHERFETCHERTEST_H
