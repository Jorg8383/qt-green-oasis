#ifndef WEATHERMODELTEST_H
#define WEATHERMODELTEST_H

#include <QObject>
#include <QTest>
#include <weatherdata.h>

class WeatherModelTest : public QObject
{
    Q_OBJECT
public:
    explicit WeatherModelTest(QObject *parent = nullptr);

signals:

private slots:
    void testRowCount();
    void testData();
};

#endif // WEATHERMODELTEST_H
