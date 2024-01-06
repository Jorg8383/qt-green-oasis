#include "logger.h"

Logger::Logger()
{

}

Logger::~Logger()
{

}

Logger &Logger::instance()
{

}

void Logger::log(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{

}

void Logger::readConfiguration()
{
    m_logToFileEnabled = ConfigManager::instance().getValue("Logging/LogToFile").toBool();
    m_logToConsoleEnabled = ConfigManager::instance().getValue("Logging/LogToConsole").toBool();
    QString filePath = ConfigManager::instance().getValue("Logging/FilePath").toString();
    m_logFile.setFileName(filePath);

}

QString Logger::logLevelToString(QtMsgType type)
{

}
