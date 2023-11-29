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
    weatherDataList.append(new WeatherData());
    model.setWeatherData(weatherDataList);

    QCOMPARE(model.rowCount(), 2);
}

void WeatherModelTest::testData()
{
//    WeatherModel model;
//    QList<WeatherData*> weatherDataList;

//    // Create a new weather data object
//    WeatherData* data = new WeatherData();
//    data->setCity("Dublin");
//    data->setMaxTemperature(6.5);
//    data->setMinTemperature(-3.5);
//    weatherDataList.append(data);

//    // Create another weather data object
//    data = new WeatherData();
//    data->setCity("Dublin");
//    data->setMaxTemperature(3.5);
//    data->setMinTemperature(-2.5);
//    weatherDataList.append(data);

//    // Pass the weather data list to the model
//    model.setWeatherData(weatherDataList);

//    QCOMPARE(model.data(model.index(0), WeatherModel::CityRole), QString("Dublin"));
//    QCOMPARE(model.data(model.index(0), WeatherModel::MaxTemperatureRole), 6.5);
//    QCOMPARE(model.data(model.index(0), WeatherModel::MinTemperatureRole), -3.5);
//    QCOMPARE(model.data(model.index(1), WeatherModel::CityRole), QString("Dublin"));
//    QCOMPARE(model.data(model.index(1), WeatherModel::MaxTemperatureRole), 3.5);
//    QCOMPARE(model.data(model.index(1), WeatherModel::MinTemperatureRole), -2.5);

}
