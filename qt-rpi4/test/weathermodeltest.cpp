#include "weathermodeltest.h"

WeatherModelTest::WeatherModelTest(QObject *parent)
    : QObject{parent}
{

}

void WeatherModelTest::testRowCount()
{
    WeatherModel model;
    QCOMPARE(model.rowCount(), 0);

    model.addWeatherData(new WeatherData());
    QCOMPARE(model.rowCount(), 1);
}

void WeatherModelTest::testData()
{
    WeatherModel model;
    WeatherData* data = new WeatherData(); // The model will handle the memory management of this object
    data->setCity("Dublin");
    model.addWeatherData(data);

    QCOMPARE(model.data(model.index(0), WeatherModel::CityRole.toString()), QString("Dublin"));
}
