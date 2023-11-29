#include "weathermodel.h"

WeatherModel::WeatherModel(QObject *parent)
    : QAbstractListModel{parent}
{

}

int WeatherModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_data.count();
}

QVariant WeatherModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= m_data.count())
        return QVariant(); // Constructs and returns an invalid variant

    WeatherData* data = m_data[index.row()];
    switch (role) {
    case CityRole:
        return data->city();
    case WeatherDescriptionRole:
        return data->weatherDescription();
    case WeatherMainRole:
        return data->weatherMain();
    case MinTemperatureRole:
        return data->minTemperature();
    case MaxTemperatureRole:
        return data->maxTemperature();
    case HumidityRole:
        return data->humidity();
    case WindSpeedRole:
        return data->windSpeed();
    case WeatherIconRole:
        return data->weatherIcon();
    default:
        return QVariant(); // Constructs and returns an invalid variant
    }

}

QHash<int, QByteArray> WeatherModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CityRole] = "city";
    roles[WeatherDescriptionRole] = "weatherDescription";
    roles[WeatherMainRole] = "weatherMain";
    roles[MinTemperatureRole] = "minTemperature";
    roles[MaxTemperatureRole] = "maxTemperature";
    roles[HumidityRole] = "humidity";
    roles[WindSpeedRole] = "windSpeed";
    roles[WeatherIconRole] = "weatherIcon";
    return roles;
}

void WeatherModel::setWeatherData(QList<WeatherData *> newData)
{
    beginResetModel(); // Any views attached to this model will be reset as well

    // Delete all old data entries
    qDeleteAll(m_data.begin(), m_data.end());

    // Replace the old data with new data
    m_data = newData;

    endResetModel();
    emit countChanged(rowCount());
}
