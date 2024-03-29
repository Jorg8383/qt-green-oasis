#include "weatherfetcher.h"

WeatherFetcher::WeatherFetcher(QNetworkAccessManager *networkManager, WeatherModel &model, QString apiKey, QObject *parent)
    : QObject{parent}, m_networkManager{networkManager}, m_weatherModel{model}, m_apiKey{apiKey}
{
    setObjectName("WeatherFetcher");
    qDebug() << this << "object is being constructed";
    // Create an interval timer and connect it to the fetchWeatherData slot
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &WeatherFetcher::fetchWeatherData);
}

WeatherFetcher::~WeatherFetcher()
{
    qDebug() << this << "object is being destroyed";
}

void WeatherFetcher::fetchWeatherData()
{
    qDebug() << this << "fetchWeatherData() is being invoked";
    // Create API URL string by replacing placeholders in string with arguments
    QString apiString = m_apiString.arg(m_latitude).arg(m_longitude).arg(m_apiKey);
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

void WeatherFetcher::startFetching(int interval)
{
    qDebug() << this << "startFetching() with an interval of" << interval << "milliseconds";
    m_timer->start(interval); // Interval in milliseconds
}

void WeatherFetcher::stopFetching()
{
    qDebug() << this << "stopFetching() is being invoked";
    m_timer->stop();
}

QNetworkRequest WeatherFetcher::createWeatherRequest(QString url)
{
    m_apiUrl.setUrl(url);
    QNetworkRequest request(m_apiUrl);
    qDebug() << this << "Weather request was created with URL: " << m_apiUrl.toString();
    return request;
}

void WeatherFetcher::clearPreviousWeatherRequest()
{
    if (m_lastReply) delete m_lastReply;
}

void WeatherFetcher::sendWeatherRequest(const QNetworkRequest &request)
{
    qDebug() << this << "sendWeatherRequest() is being invoked";
    m_lastReply = m_networkManager->get(request);
    m_lastReply->setParent(this);
    connect(m_lastReply, &QNetworkReply::finished, this, &WeatherFetcher::exractWeatherFromReply);
}

QJsonObject WeatherFetcher::extractJsonFromReply()
{
    qDebug() << this << "extractJsonFromReply() is being invoked";
    // First, check for null pointers
    if (!m_lastReply)
    {
        throw std::runtime_error("WeatherFetcher::extractJsonFromReply() - m_lastReply pointer is null!");
    }

    // Read the received JSON data
    const QByteArray data = m_lastReply->readAll();

    // Convert the received JSON data into a QJsonObject
    QJsonParseError parseError;
    QJsonObject jsonObj = QJsonDocument::fromJson(data, &parseError).object();

    // Check for potential errors
    if (parseError.error != QJsonParseError::NoError)
    {
        // Report a warning about the parsing error
        qWarning() << this << "Error: JSON parsing failed: " << parseError.errorString();
        // Emit an error signal with details
        emit networkError(QNetworkReply::UnknownContentError, parseError.errorString());
    }
    else if (jsonObj.isEmpty()) {
        // Report a warning about the empty object
        qWarning() << this << "Error: JSON object is empty!";
    }

    return jsonObj;
}

bool WeatherFetcher::requestWasSuccessful()
{
    // First, check for null pointers
    if (!m_lastReply)
    {
        throw std::runtime_error("WeatherFetcher::requestWasSuccessful() - m_lastReply pointer is null!");
    }

    // Check weather the reply was ok
    bool status = true;
    if (!(m_lastReply->error() == QNetworkReply::NoError
          && m_lastReply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 200)) // 200 == ok
    {
        // Report a warning about the occured network error
        qWarning() << this << "Network error occured: " << m_lastReply->errorString();
        // Emit an error signal with details
        emit networkError(m_lastReply->error(), m_lastReply->errorString());
        status = false;
    }
    qDebug() << this << "requestWasSuccessful() returned status: " << status;
    return status;
}

void WeatherFetcher::extractWeatherFromJson(const QJsonObject &json)
{
    qDebug() << this << "extractWeatherFromJson(...) is being invoked";
    if (json.isEmpty())
    {
        qWarning() << "Error: weather can't be extracted from JSON due to a empty JSON object!";
        return;
    }

    // Create a weather data list that can be passed to the weather model
    QList<WeatherData*> weatherItemList;

    // Extract "city" object information
    QJsonObject cityObject = json["city"].toObject();
    QString cityName = cityObject["name"].toString();
    qDebug() << this << "Extracted city name: " << cityName;

    // Extract weather information
    bool isCurrentWeather = true;
    QJsonArray weatherInfoList = json["list"].toArray();
    for (const QJsonValue& listValue: weatherInfoList)
    {
        if (listValue.isObject())
        {
            QJsonObject listObject = listValue.toObject();
            QString listItemName = listObject["dt_txt"].toString();
            WeatherData *weatherDataItem = new WeatherData(listItemName, listObject, cityName, isCurrentWeather);
            isCurrentWeather = false;

            // Append each weather data item to the list
            weatherItemList.append(weatherDataItem);
        }
    }

    // Pass the created weather item list to the weather model
    m_weatherModel.setWeatherData(weatherItemList);
}

double WeatherFetcher::latitude() const
{
    return m_latitude;
}

void WeatherFetcher::setLatitude(double newLatitude)
{
    m_latitude = newLatitude;
}

double WeatherFetcher::longitude() const
{
    return m_longitude;
}

void WeatherFetcher::setLongitude(double newLongitude)
{
    m_longitude = newLongitude;
}

void WeatherFetcher::exractWeatherFromReply()
{
    qDebug() << this << "exractWeatherFromReply() is being invoked";
    if (requestWasSuccessful())
    {
        const QJsonObject jsonObj = extractJsonFromReply();
        extractWeatherFromJson(jsonObj);
        emit dataUpdated();
    }

}

QUrl WeatherFetcher::apiUrl() const
{
    return m_apiUrl;
}


