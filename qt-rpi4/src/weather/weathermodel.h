#ifndef WEATHERMODEL_H
#define WEATHERMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "weatherdata.h"

class WeatherModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
public:
    explicit WeatherModel(QObject *parent = nullptr);

    enum Roles {
        CityRole = Qt::UserRole + 1,
        WeatherDescriptionRole,
        WeatherMainRole,
        MinTemperatureRole,
        MaxTemperatureRole,
        HumidityRole,
        WindSpeedRole,
        WeatherIconRole
        // Add other roles as needed
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWeatherData(QList<WeatherData*> newData);

signals:
    void countChanged(int count);

private:
    QList<WeatherData*> m_data;
};

#endif // WEATHERMODEL_H