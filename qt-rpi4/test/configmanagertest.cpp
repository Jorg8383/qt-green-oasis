#include "configmanagertest.h"

ConfigManagerTest::ConfigManagerTest(QObject *parent)
    : QObject{parent}
{
    setObjectName("ConfigManagerTest");
}

void ConfigManagerTest::initTestCase()
{
    // Create a temporary config.txt file and fill it with sample data
    QFile configFile("temp_config.txt");
    if (configFile.open(QIODevice::WriteOnly))
    {
        QTextStream stream(&configFile);
        stream << "[Database]\n";
        stream << "databaseName=myDatabase\n";
        stream << "databaseUser=myUser\n";
        stream << "databasePassword=myPassword\n";
        stream << "[Server]\n";
        stream << "serverPort=8080\n";
        stream << "[Logging]\n";
        stream << "logLevel=INFO";
        configFile.close();
    }
}

void ConfigManagerTest::cleanupTestCase()
{
    // Delete temporary config file
    QFile::remove("temp_config.txt");
}

void ConfigManagerTest::testGetValue()
{

    // Add test cases...
    ConfigManager configManager("temp_config.txt");
    QCOMPARE(configManager.getValue("Database/databaseName").toString(), QString("myDatabase"));
    QCOMPARE(configManager.getValue("Database/databaseUser").toString(), QString("myUser"));
    QCOMPARE(configManager.getValue("Database/databasePassword").toString(), QString("myPassword"));
    QCOMPARE(configManager.getValue("Server/serverPort").toInt(), 8080);
    QCOMPARE(configManager.getValue("Logging/logLevel").toString(), QString("INFO"));
}

void ConfigManagerTest::testKeyNotFound()
{
    ConfigManager configManager("temp_config.txt");
    // Check for the "logLevel" key without providing the section as a prefix
    QString value = configManager.getValue("logLevel").toString();
    QVERIFY(value.isEmpty());
}
