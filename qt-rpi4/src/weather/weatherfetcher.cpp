#include "weatherfetcher.h"

WeatherFetcher::WeatherFetcher(WeatherModel &model, const QString &apiKey, QObject *parent)
    : QObject{parent}, m_weatherModel{model}, m_apiKey(apiKey)
{

}

WeatherFetcher::~WeatherFetcher()
{

}

void WeatherFetcher::getData(const double latitude, const double longitude)
{

}
