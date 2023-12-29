#include "mocknetworkreply.h"

MockNetworkReply::MockNetworkReply(const QByteArray &data, QObject *parent)
    : QNetworkReply(parent)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    setReadBufferSize(data.size());
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    write(data);
    seek(0);
}
