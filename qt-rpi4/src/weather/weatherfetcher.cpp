#include "weatherfetcher.h"

WeatherFetcher::WeatherFetcher(WeatherModel &model, const QString &apiKey, QObject *parent)
    : QObject{parent}, m_weatherModel{model}, m_apiKey(apiKey)
{
    // Create signal and slot connections between this class and the network access manager
    connect(&m_networkManager, &QNetworkAccessManager::finished, this, &WeatherFetcher::replyFinished);
}

WeatherFetcher::~WeatherFetcher()
{

}

void WeatherFetcher::getData(const double latitude, const double longitude)
{
    qDebug() << "Firing off GET request to openweather...";
    // api.openweathermap.org/data/2.5/forecast?lat={lat}&lon={lon}&appid={API key}
    QString apiString = QString("https://api.openweathermap.org/data/2.5/forecast?lat=%1&lon=%2&appid=%3")
                            .arg(latitude).arg(longitude).arg(m_apiKey);
    qDebug() << "openeweather API string: " << apiString;
    QUrl apiUrl(apiString);
    QNetworkRequest request(apiUrl);
    m_networkManager.get(request);
}

void WeatherFetcher::replyFinished(QNetworkReply *reply)
{

}
