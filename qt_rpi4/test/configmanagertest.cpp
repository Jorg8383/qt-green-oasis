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

    // Set the configuration file for the singleton instance
    ConfigManager::instance().initialise("temp_config.txt");
}

void ConfigManagerTest::cleanupTestCase()
{
    // Delete temporary config file
    QFile::remove("temp_config.txt");
}

void ConfigManagerTest::testGetValue()
{

    QCOMPARE(ConfigManager::instance().getValue("Database/databaseName").toString(), QString("myDatabase"));
    QCOMPARE(ConfigManager::instance().getValue("Database/databasePassword").toString(), QString("myPassword"));
    QCOMPARE(ConfigManager::instance().getValue("Server/serverPort").toInt(), 8080);
    QCOMPARE(ConfigManager::instance().getValue("Database/databaseUser").toString(), QString("myUser"));
    QCOMPARE(ConfigManager::instance().getValue("Logging/logLevel").toString(), QString("INFO"));
    // Add test cases...
}

void ConfigManagerTest::testKeyNotFound()
{
    // Check for the "logLevel" key without providing the section as a prefix
    QString value = ConfigManager::instance().getValue("logLevel").toString();
    QVERIFY(value.isEmpty());
}

void ConfigManagerTest::testFileOpenError()
{
    ConfigManager& configManager = ConfigManager::instance();
    QString nonExistentFile = "nonexistentfile.txt";

    // Check that a std::runtime_error is thrown when trying to open a non-existent file
    QVERIFY_EXCEPTION_THROWN(configManager.initialise(nonExistentFile), std::runtime_error);

}
