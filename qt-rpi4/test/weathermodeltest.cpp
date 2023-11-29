#include "weathermodeltest.h"

WeatherModelTest::WeatherModelTest(QObject *parent)
    : QObject{parent}
{

}

void WeatherModelTest::testRowCount()
{
    WeatherModel model;
    QCOMPARE(model.rowCount(), 0);

    QList<WeatherData*> weatherDataList;
    weatherDataList.append(new WeatherData());

    QCOMPARE(model.rowCount(), 1);
}

void WeatherModelTest::testData()
{
    WeatherModel model;

    // Create a new weather data object
    WeatherData* data = new WeatherData();
    data->setCity("Dublin");

    // Add this object to a list of weather data objects
    QList<WeatherData*> weatherDataList;
    weatherDataList.append(data);
    model.setWeatherData(weatherDataList);

    QCOMPARE(model.data(model.index(0), WeatherModel::CityRole), QString("Dublin"));
}
