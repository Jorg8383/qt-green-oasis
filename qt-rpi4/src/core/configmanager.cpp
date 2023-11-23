#include "configmanager.h"

ConfigManager::ConfigManager(const QString &configFileName, QObject *parent)
    : QObject{parent}
{

}

QVariant ConfigManager::getValue(const QString &key, bool fromEnv) const
{
    return 0;
}
