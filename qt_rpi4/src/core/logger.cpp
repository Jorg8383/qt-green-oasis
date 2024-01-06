#include "logger.h"

Logger::Logger()
{
    readConfiguration();
    if (m_logToFileEnabled)
    {
        if (!m_logFile.open(QIODevice::Append | QIODevice::Text))
        {
            qWarning() << "Failed to open log file:" << m_logFile.errorString();
            m_logToFileEnabled = false;
        }
    }
}

Logger::~Logger()
{
    if (m_logFile.isOpen())
        m_logFile.close();
}

Logger &Logger::instance()
{
    static Logger loggerInstance;
    return loggerInstance;
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
    switch(type)
    {
    case QtDebugMsg: return "DEBUG";
    case QtInfoMsg: return "INFO";
    case QtWarningMsg: return "WARNING";
    case QtCriticalMsg: return "CRITICAL";
    case QtFatalMsg: return "FATAL";
    default: return "UNKNOWN";
    }
}
