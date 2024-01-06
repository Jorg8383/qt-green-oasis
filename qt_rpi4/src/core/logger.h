#ifndef LOGGER_H
#define LOGGER_H

#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QMutex>
#include <QMutexLocker>
#include <QDebug>
#include <QSettings>

class Logger
{
public:
    // The logger uses the singleton pattern, so the constructor has to be private
    static Logger& instance();
    void log(QtMsgType type, const QMessageLogContext &context, const QString &msg);

private:
    Logger();
    ~Logger();
    void readConfiguration();
    static QString logLevelToString(QtMsgType type);

    QFile logFile;
    bool logToFileEnabled;
    bool logToConsoleEnabled;
    QMutex mutex;
};

#endif // LOGGER_H
