#ifndef WEATHERMODEL_H
#define WEATHERMODEL_H

#include <QObject>
#include <QAbstractListModel>
#include "weatherdata.h"

class WeatherModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)

    Q_PROPERTY(QString currentCityName READ currentCityName NOTIFY currentDataChanged)
    Q_PROPERTY(QString currentWeatherDescription READ currentWeatherDescription NOTIFY currentDataChanged)
    Q_PROPERTY(QString currentWeatherIcon READ currentWeatherIcon NOTIFY currentDataChanged)
    Q_PROPERTY(double currentMainTemp READ currentMainTemp NOTIFY currentDataChanged)
    Q_PROPERTY(double currentWindSpeed READ currentWindSpeed NOTIFY currentDataChanged)
    Q_PROPERTY(double currentPop READ currentPop NOTIFY currentDataChanged)


public:
    explicit WeatherModel(QObject *parent = nullptr);
    ~WeatherModel(); // Deconstructor

    enum Roles {
        CityNameRole = Qt::UserRole + 1,
        IsCurrentWeatherRole,
        DateAndTimeRole,
        WeatherDescriptionRole,
        WeatherMainRole,
        WeatherIconRole,
        TemperatureRole,
        MinTemperatureRole,
        MaxTemperatureRole,
        WindSpeedRole,
        Rain3hRole,
        Snow3hRole,
        PopRole
        // Add other roles as needed
    };

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void setWeatherData(QList<WeatherData*> newData);

    QString currentCityName() const;
    QString currentWeatherDescription() const;
    QString currentWeatherIcon() const;
    double currentMainTemp() const;
    double currentWindSpeed() const;
    double currentPop() const;

signals:
    void countChanged(int count);
    void currentDataChanged();

private:
    QList<WeatherData*> m_data;
};

#endif // WEATHERMODEL_H
