#ifndef CUSTOMMESSAGEHANDLER_H
#define CUSTOMMESSAGEHANDLER_H

#include <QtGlobal>
#include <QMessageLogContext>
#include <QString>
#include <logger.h>

inline void customMessageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
    Logger::instance().log(type, context, msg);
}

#endif // CUSTOMMESSAGEHANDLER_H
