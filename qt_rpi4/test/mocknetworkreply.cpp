#include "mocknetworkreply.h"


MockNetworkReply::MockNetworkReply(const QByteArray &data, QObject *parent)
    : QNetworkReply(parent), m_data(data)
{
    setOperation(QNetworkAccessManager::GetOperation);
    setAttribute(QNetworkRequest::HttpStatusCodeAttribute, 200);
    setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    setHeader(QNetworkRequest::ContentLengthHeader, data.size());
    setReadBufferSize(data.size());
    open(QIODevice::ReadOnly | QIODevice::Unbuffered);
    // seek(0);
}

QSharedPointer<QNetworkReply> MockNetworkReply::createMockNetworkReply(const QByteArray &data)
{
    return QSharedPointer<QNetworkReply>(new MockNetworkReply(data));
}

qint64 MockNetworkReply::readData(char *data, qint64 maxlen)
{
    qint64 bytesToRead = qMin(maxlen, static_cast<qint64>(m_data.size()) - pos());
    if (bytesToRead <= 0)
        return -1;

    memcpy(data, m_data.constData() + pos(), bytesToRead);
    return bytesToRead;
}

void MockNetworkReply::abort()
{

}
