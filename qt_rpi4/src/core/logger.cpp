#include "logger.h"

Logger::Logger()
{
    setObjectName("Logger");
    readConfiguration();
    if (m_logToFileEnabled)
    {
        if (!m_logFile.open(QIODevice::Append | QIODevice::Text))
        {
            qWarning() << "Failed to open log file:" << m_logFile.errorString();
            m_logToFileEnabled = false;
        } else
        {
            qInfo() << this << "Log file has been opened successfully";
            qDebug() << "Log file: " << m_logFile.fileName();
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

    // Create a formatted message that is configurable
    QString formattedMessage = QString("%1 [%2]").arg(timeStamp, logLevel);
    if (m_logFileAndLineEnabled)
    {
        formattedMessage += QString(" (%1:%2)").arg(QString::fromUtf8(context.file),
                                                    QString::number(context.line));
    }
    if (m_logContentEnabled)
    {
        formattedMessage += QString(": %1").arg(msg);
    }

    // Log to the file if enabled
    if (m_logToFileEnabled && m_logFile.isOpen())
    {
        QTextStream out(&m_logFile);
        out << formattedMessage << "\n";
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
    // Get config flags from the config.ini file
    m_logToFileEnabled = ConfigManager::instance().getValue("Logging/LogToFile").toBool();
    m_logToConsoleEnabled = ConfigManager::instance().getValue("Logging/LogToConsole").toBool();
    m_logFileAndLineEnabled = ConfigManager::instance().getValue("Logging/LogFileAndLineEnabled").toBool();
    m_logContentEnabled = ConfigManager::instance().getValue("Logging/LogContentEnabled").toBool();

    // Use the temp directory for the log file
    QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QString logFileName = ConfigManager::instance().getValue("Logging/FileName").toString();
    QString logFilePath = QDir(appDataPath).filePath(logFileName);

    if (m_logToFileEnabled) {
        m_logFile.setFileName(logFilePath);
        // qDebug() << "Log file path" << logFilePath;
    }
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
