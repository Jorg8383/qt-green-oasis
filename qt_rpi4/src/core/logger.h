#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include <QSettings>
#include <QStandardPaths>
#include <configmanager.h>

class Logger : public QObject
{
    Q_OBJECT
public:
    // The logger uses the singleton pattern, so the constructor has to be private
    static Logger& instance();
    void log(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    Logger();
    ~Logger();
    void readConfiguration();
    static QString logLevelToString(QtMsgType type);

    QFile m_logFile;
    bool m_logToFileEnabled;
    bool m_logToConsoleEnabled;
    bool m_logFileAndLineEnabled;
    bool m_logContentEnabled;
    QMutex m_mutex;
};

#endif // LOGGER_H
