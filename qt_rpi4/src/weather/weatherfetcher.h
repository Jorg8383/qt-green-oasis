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
#include <QTimer>
#include <stdexcept>
#include "weathermodel.h"
#include "weatherdata.h"

class WeatherFetcher : public QObject
{
    Q_OBJECT
public:
    explicit WeatherFetcher(QNetworkAccessManager* networkManager, WeatherModel& model, QString apiKey, QObject *parent = nullptr);
    ~WeatherFetcher(); // Deconstructor
    bool fetchIsFinished() const;
    void startFetching(int interval); // Interval in milliseconds
    void stopFetching();

    QUrl apiUrl() const;

    double longitude() const;
    void setLongitude(double newLongitude);

    double latitude() const;
    void setLatitude(double newLatitude);

signals:
    void dataUpdated();
    void networkError(QNetworkReply::NetworkError errorCode, const QString& errorString);

public slots:
    void fetchWeatherData();

private slots:
    void exractWeatherFromReply();

private:
    QNetworkRequest createWeatherRequest(QString url);
    void clearPreviousWeatherRequest();
    void sendWeatherRequest(const QNetworkRequest& request);
    QJsonObject extractJsonFromReply();
    bool requestWasSuccessful();
    void extractWeatherFromJson(const QJsonObject& json);

    // Private members
    QTimer* m_timer;
    QPointer<QNetworkAccessManager> m_networkManager;
    QPointer<QNetworkReply> m_lastReply;
    WeatherModel& m_weatherModel;
    QString m_apiKey;
    QString m_apiString = "https://api.openweathermap.org/data/2.5/forecast?lat=%1&lon=%2&appid=%3&units=metric";
    QUrl m_apiUrl;
    double m_longitude;
    double m_latitude;
};

#endif // WEATHERFETCHER_H
