#include "weatherfetcher.h"

WeatherFetcher::WeatherFetcher(WeatherModel &model, const QString &apiKey, QObject *parent)
    : QObject{parent}, m_weatherModel{model}, m_apiKey(apiKey)
{
    setObjectName("WeatherFetcher");
    // Create signal and slot connections between this class and the network access manager
    connect(&m_networkManager, &QNetworkAccessManager::finished, this, &WeatherFetcher::replyFinished);
}

WeatherFetcher::~WeatherFetcher()
{

}

void WeatherFetcher::getData(const double latitude, const double longitude)
{
    qDebug() << this << " - Firing off GET request to openweather...";
    // api.openweathermap.org/data/2.5/forecast?lat={lat}&lon={lon}&appid={API key}
    QString apiString = QString("https://api.openweathermap.org/data/2.5/forecast?lat=%1&lon=%2&appid=%3&units=metric")
                            .arg(latitude).arg(longitude).arg(m_apiKey);
    qDebug() << this << " - Created openeweather API string: " << apiString;
    QUrl apiUrl(apiString);
    QNetworkRequest request(apiUrl);
    m_networkManager.get(request);
}

void WeatherFetcher::replyFinished(QNetworkReply *reply)
{
    qDebug() << this << " - replyFinished() is being invoked...";
    if (reply->error() != QNetworkReply::NoError)
    {
        // Report a warning about the occured network error
        qWarning() << this << " - Network error: " << reply->errorString();
        // Emit an error signal with details
        emit networkError(reply->error(), reply->errorString());
    }
    else
    {
        // Read the received JSON data
        QByteArray data = reply->readAll();

        // Convert the received JSON data into a QJsonDocument
        QJsonParseError parseError;
        QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &parseError);

        // Check for parsing errors
        if (parseError.error != QJsonParseError::NoError)
        {
            // Report a warning about the parsing error
            qWarning() << this << " - JSON parsing error: " << parseError.errorString();
            // Emit an error signal with details
            emit networkError(QNetworkReply::UnknownContentError, parseError.errorString());
            return;
        }

        // Extract weather information
        if (jsonResponse.isObject())
        {
            extractWeatherInfo(jsonResponse);
        }
        else
        {
            qWarning() << this << " - invalid data; JSON object was expected";
        }
    }
}

void WeatherFetcher::extractWeatherInfo(const QJsonDocument &jsonResponse)
{
    QList<WeatherData*> weatherItemList;

    QJsonObject rootObject = jsonResponse.object();

    // Extract "city" object information
    QJsonObject cityObject = rootObject["city"].toObject();
    QString cityName = cityObject["name"].toString();

    // Extract weather information
    QJsonArray weatherInfoList = rootObject["list"].toArray();
    for (const QJsonValue& listValue: weatherInfoList)
    {
        if (listValue.isObject())
        {
//            WeatherData *dataEntry = new WeatherData();
//            dataEntry->setCity(cityName);

//            QJsonObject listObject = listValue.toObject();

//            dataEntry->setDt(listObject["dt"].toInt());
//            dataEntry->setObjectName(listObject["dt_txt"].toString());

//            QJsonObject mainObject = listObject["main"].toObject();

        }
    }
}
