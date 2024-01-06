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
    // Lock the mutex to ensure thread-safety
    QMutexLocker locker(&m_mutex);
    // Create the time stamp and the log level as strings
    QString timeStamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz");
    QString logLevel = logLevelToString(type);
    // Create a formatted message that includes all details required to track down the source
    QString formattedMessage = QString("%1 [%2] (%3:%4, %5): %6")
                                   .arg(timeStamp,
                                        logLevel,
                                        QString::fromUtf8(context.file),
                                        QString::number(context.line),
                                        QString::fromUtf8(context.function),
                                        msg);

    // Log to the file if enabled
    if (m_logToFileEnabled && m_logFile.isOpen())
    {
        QTextStream out(&m_logFile);
        out << formattedMessage;
        out.flush();
    }

    // Log to the console if enabled
    if (m_logToConsoleEnabled)
    {
        fprintf(stderr, "%s\n", formattedMessage.toLocal8Bit().constData());
    }
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
