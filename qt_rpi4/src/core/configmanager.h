#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QFile>
#include <QTextStream>
#include <QMap>
#include <QVariant>
#include <QDebug>

class ConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManager(const QString &configFileName = "", QObject *parent = nullptr);

    QVariant getValue(const QString &key) const;

signals:

private:
    QMap<QString, QVariant> m_configData;
};

#endif // CONFIGMANAGER_H
