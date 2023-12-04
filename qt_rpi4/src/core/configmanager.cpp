#include "configmanager.h"

ConfigManager::ConfigManager(const QString &configFileName, QObject *parent)
    : QObject{parent}
{
    setObjectName("ConfigManager");

    QFile configFile(configFileName);
    if (configFile.open(QIODevice::ReadOnly))
    {
        QTextStream stream(&configFile);
        QString currentSection;
        while (!stream.atEnd())
        {
            QString line = stream.readLine().trimmed();
            if (line.startsWith("[") && line.endsWith("]"))
            {
                // Set current section
                currentSection = line.mid(1, line.length() - 2);
            }
            else
            {
                // Split the line into key/value pairs
                QStringList parts = line.split('=');
                if (parts.count() == 2)
                {
                    // Use the ternary operator to handle keys with and without sections
                    QString key = currentSection.isEmpty() ? parts[0].trimmed() :
                                      currentSection + "/" + parts[0].trimmed();
                    m_configData.insert(key, parts[1].trimmed());
                }
            }
        }
        configFile.close();
    }
}

QVariant ConfigManager::getValue(const QString &key) const
{
    if (!m_configData.contains(key)) qWarning() << this << "key not found: " << key;
    return m_configData.value(key);
}
