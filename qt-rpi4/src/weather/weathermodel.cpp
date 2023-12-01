#include "weathermodel.h"

WeatherModel::WeatherModel(QObject *parent)
    : QAbstractListModel{parent}
{
    setObjectName("WeatherModel");
}

WeatherModel::~WeatherModel()
{
    // Delete all data entries
    qDeleteAll(m_data.begin(), m_data.end());
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
    case CityNameRole:
        return data->cityName();
    case WeatherDescriptionRole:
        return data->weatherDescription();
    case WeatherMainRole:
        return data->weatherMain();
    case TemperatureRole:
        return data->mainTemp();
    case MinTemperatureRole:
        return data->mainTempMin();
    case MaxTemperatureRole:
        return data->mainTempMax();
    case WindSpeedRole:
        return data->windSpeed();
    case WeatherIconRole:
        return data->weatherIcon();
    case Rain3hRole:
        return data->rain3h();
    case Snow3hRole:
        return data->snow3h();
    case PopRole:
        return data->pop();
    default:
        return QVariant(); // Constructs and returns an invalid variant
    }

}

QHash<int, QByteArray> WeatherModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[CityNameRole] = "cityName";
    roles[WeatherDescriptionRole] = "weatherDescription";
    roles[WeatherMainRole] = "weatherMain";
    roles[WeatherIconRole] = "weatherIcon";
    roles[TemperatureRole] = "mainTemp";
    roles[MinTemperatureRole] = "mainTempMin";
    roles[MaxTemperatureRole] = "mainTempMax";
    roles[WindSpeedRole] = "windSpeed";
    roles[Rain3hRole] = "rain3h";
    roles[Snow3hRole] = "snow3h";
    roles[PopRole] = "pop";
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
