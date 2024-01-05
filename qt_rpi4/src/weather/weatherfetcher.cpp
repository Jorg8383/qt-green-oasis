#include "weatherfetcher.h"

WeatherFetcher::WeatherFetcher(QNetworkAccessManager *networkManager, WeatherModel &model, QString apiKey, QObject *parent)
    : QObject{parent}, m_networkManager{networkManager}, m_weatherModel{model}, m_apiKey{apiKey}
{
    setObjectName("WeatherFetcher");
    // Create signal and slot connections between this class and the network access manager
    // connect(&m_networkManager, &QNetworkAccessManager::finished, this, &WeatherFetcher::replyFinished);
}

WeatherFetcher::~WeatherFetcher()
{

}

void WeatherFetcher::fetchData(const double latitude, const double longitude)
{
    // Create API URL string by replacing placeholders in string with arguments
    QString apiString = m_apiString.arg(latitude).arg(longitude).arg(m_apiKey);
    if (m_networkManager)
    {
        clearPreviousWeatherRequest();
        const QNetworkRequest weatherRequest = createWeatherRequest(apiString);
        sendWeatherRequest(weatherRequest);
    }
}

bool WeatherFetcher::fetchIsFinished() const
{
    return m_lastReply && m_lastReply->isFinished();
}

QNetworkRequest WeatherFetcher::createWeatherRequest(QString url)
{
    m_apiUrl.setUrl(url);
    QNetworkRequest request(m_apiUrl);
    qDebug() << this << "created weather request with URL: " << m_apiUrl.toString();
    return request;
}

void WeatherFetcher::clearPreviousWeatherRequest()
{
    if (m_lastReply) delete m_lastReply;
}

void WeatherFetcher::sendWeatherRequest(const QNetworkRequest &request)
{
    m_lastReply = m_networkManager->get(request);
    m_lastReply->setParent(this);
    connect(m_lastReply, &QNetworkReply::finished, this, &WeatherFetcher::exractWeatherFromReply);
}

QJsonObject WeatherFetcher::extractJsonFromReply()
{
    // Read the received JSON data
    const QByteArray data = m_lastReply->readAll();

    // Convert the received JSON data into a QJsonObject
    QJsonParseError parseError;
    QJsonObject jsonObj = QJsonDocument::fromJson(data, &parseError).object();

    // Check for potential errors
    if (parseError.error != QJsonParseError::NoError)
    {
        // Report a warning about the parsing error
        qWarning() << this << " - JSON parsing error: " << parseError.errorString();
        // Emit an error signal with details
        emit networkError(QNetworkReply::UnknownContentError, parseError.errorString());
    }
    else if (jsonObj.isEmpty()) {
        // Report a warning about the empty object
        qWarning() << this << " - JSON object is empty!";
    }

    return jsonObj;
}

bool WeatherFetcher::requestWasSuccessful()
{

    // First, check for null pointers
    if (!m_lastReply)
    {
        throw std::runtime_error("Weatherfetcher - m_lastReply object is null!");
    }

    // Check weather the reply was ok
    bool status = true;
    if (!(m_lastReply->error() == QNetworkReply::NoError
          && m_lastReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)) // 200 == ok
    {
        // Report a warning about the occured network error
        qWarning() << this << "network error occured: " << m_lastReply->errorString();
        // Emit an error signal with details
        emit networkError(m_lastReply->error(), m_lastReply->errorString());
        status = false;
    }
    return status;
}

void WeatherFetcher::extractWeatherFromJson(const QJsonObject &json)
{
    // TODO extract weather details here...
}

void WeatherFetcher::exractWeatherFromReply()
{
    if (requestWasSuccessful())
    {
        const QJsonObject jsonObj = extractJsonFromReply();
        if (!jsonObj.isEmpty())
            extractWeatherFromJson(jsonObj);
    }

}

// void WeatherFetcher::replyFinished(QNetworkReply *reply)
// {
//     qDebug() << this << " - replyFinished() is being invoked...";
//     if (reply->error() != QNetworkReply::NoError)
//     {
//         // Report a warning about the occured network error
//         qWarning() << this << " - Network error: " << reply->errorString();
//         // Emit an error signal with details
//         emit networkError(reply->error(), reply->errorString());
//     }
//     else
//     {
//         // Read the received JSON data
//         QByteArray data = reply->readAll();

//         // Convert the received JSON data into a QJsonDocument
//         QJsonParseError parseError;
//         QJsonDocument jsonResponse = QJsonDocument::fromJson(data, &parseError);

//         // Check for parsing errors
//         if (parseError.error != QJsonParseError::NoError)
//         {
//             // Report a warning about the parsing error
//             qWarning() << this << " - JSON parsing error: " << parseError.errorString();
//             // Emit an error signal with details
//             emit networkError(QNetworkReply::UnknownContentError, parseError.errorString());
//             return;
//         }

//         // Extract weather information
//         if (jsonResponse.isObject())
//         {
//             extractWeatherInfo(jsonResponse);
//         }
//         else
//         {
//             qWarning() << this << " - invalid data; JSON object was expected";
//         }

//         qDebug() << this << " - dataUpdated() is emitted";
//         emit dataUpdated();
//     }
// }

// void WeatherFetcher::extractWeatherInfo(const QJsonDocument &jsonResponse)
// {
//     QList<WeatherData*> weatherItemList;

//     QJsonObject rootObject = jsonResponse.object();

//     // Extract "city" object information
//     QJsonObject cityObject = rootObject["city"].toObject();
//     QString cityName = cityObject["name"].toString();

//     // Extract weather information
//     bool isCurrentWeather = true;
//     QJsonArray weatherInfoList = rootObject["list"].toArray();
//     for (const QJsonValue& listValue: weatherInfoList)
//     {
//         if (listValue.isObject())
//         {
//             QJsonObject listObject = listValue.toObject();
//             QString listItemName = listObject["dt_txt"].toString();
//             WeatherData *weatherDataItem = new WeatherData(listItemName, listObject, cityName, isCurrentWeather);
//             isCurrentWeather = false;

//             // Append each weather data item to the list
//             weatherItemList.append(weatherDataItem);
//         }
//     }

//     // Pass the created weather item list to the weather model
//     m_weatherModel.setWeatherData(weatherItemList);
// }

QUrl WeatherFetcher::apiUrl() const
{
    return m_apiUrl;
}


