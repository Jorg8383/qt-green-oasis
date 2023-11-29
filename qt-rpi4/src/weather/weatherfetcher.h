#ifndef WEATHERFETCHER_H
#define WEATHERFETCHER_H

#include <QObject>
#include <QDebug>
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
    explicit WeatherFetcher(WeatherModel& model, const QString& apiKey, QObject *parent = nullptr);
    ~WeatherFetcher(); // Deconstructor
    void getData(const double latitude, const double longitude);

signals:
    void dataUpdated();
    void networkError(QNetworkReply::NetworkError errorCode, const QString& errorString);

private slots:
    void replyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager m_networkManager;
    WeatherModel& m_weatherModel;
    const QString& m_apiKey;
};

#endif // WEATHERFETCHER_H
