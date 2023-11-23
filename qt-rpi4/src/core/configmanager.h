#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QSettings>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(const QString &configFileName = "", QObject *parent = nullptr);

    QVariant getValue(const QString &key, bool fromEnv = false) const;

signals:

private:
    QSettings m_settings;
};

#endif // CONFIGMANAGER_H
