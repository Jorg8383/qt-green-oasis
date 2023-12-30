#include "mocknetworkaccessmanager.h"
#include "mocknetworkreply.h"

MockNetworkAccessManager::MockNetworkAccessManager(const QByteArray &mockData, QObject *parent)
    : QNetworkAccessManager{parent}, m_mockData(mockData)
{

}

QNetworkReply *MockNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    qDebug() << "MockNetworkAccessManager::createRequest(...) is being invoked...";
    if (op == QNetworkAccessManager::GetOperation)
    {
        // If it's a GET operation, return a mock reply
        qDebug() << "MockNetworkAccessManager::createRequest(...) returned MockNetworkReply";
        return new MockNetworkReply(m_mockData, this);
    }
    // For all other operations, call the base implementation
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
