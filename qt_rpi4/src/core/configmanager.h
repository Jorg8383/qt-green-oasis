#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QMap>
#include <QVariant>
#include <QDebug>
#include <stdexcept>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    // Singleton
    static ConfigManager& instance();
    void initialise(const QString& configFileName);

    QVariant getValue(const QString &key) const;

signals:

private:
    ConfigManager(); // Private constructor to prevent instantiation
    ~ConfigManager(); // Private deconstructor

    QMap<QString, QVariant> m_configData;
};

#endif // CONFIGMANAGER_H
