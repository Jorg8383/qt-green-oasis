#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <QObject>
#include <QDebug>
#include <QPointer>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>
#include "weathermodel.h"
#include "weatherdata.h"

class WeatherFetcher : public QObject
{
    Q_OBJECT
public:
    explicit WeatherFetcher(QNetworkAccessManager* networkManager, WeatherModel& model, QString apiKey, QObject *parent = nullptr);
    ~WeatherFetcher(); // Deconstructor
    void fetchData(const double latitude, const double longitude);
    bool fetchIsFinished() const;

    QUrl apiUrl() const;

signals:
    void dataUpdated();
    void networkError(QNetworkReply::NetworkError errorCode, const QString& errorString);

private slots:
    QNetworkRequest createWeatherRequest(QString url);
    void clearPreviousWeatherRequest();
    void sendWeatherRequest(const QNetworkRequest& request);
    QJsonObject extractJsonFromReply();
    void exractWeatherFromReply();


    // void replyFinished(QNetworkReply *reply);

private:
    // void extractWeatherInfo(const QJsonDocument& jsonResponse);

    QPointer<QNetworkAccessManager> m_networkManager;
    QPointer<QNetworkReply> m_lastReply;
    WeatherModel& m_weatherModel;
    QString m_apiKey;
    QString m_apiString = "https://api.openweathermap.org/data/2.5/forecast?lat=%1&lon=%2&appid=%3&units=metric";
    QUrl m_apiUrl;
};

#endif // WEATHERFETCHER_H
