#include "configmanagertest.h"

ConfigManagerTest::ConfigManagerTest(QObject *parent)
    : QObject{parent}
{
    setObjectName("TestConfigManager");
//    qInfo() << "Constructed: " << this;
}

void ConfigManagerTest::initTestCase()
{

}

void ConfigManagerTest::cleanupTestCase()
{
    QFile::remove("temp_config.txt");
}

void ConfigManagerTest::testGetValue()
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

    // Add test cases...
    ConfigManager configManager("temp_config.txt");
    QCOMPARE(configManager.getValue("Database/databaseName").toString(), QString("myDatabase"));
    QCOMPARE(configManager.getValue("Database/databaseUser").toString(), QString("myUser"));
    QCOMPARE(configManager.getValue("Database/databasePassword").toString(), QString("myPassword"));
    QCOMPARE(configManager.getValue("Server/serverPort").toInt(), 8080);
    QCOMPARE(configManager.getValue("Logging/logLevel").toString(), QString("INFO"));
}
