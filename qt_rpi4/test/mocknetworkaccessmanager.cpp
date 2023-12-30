#include "mocknetworkaccessmanager.h"
#include "mocknetworkreply.h"

MockNetworkAccessManager::MockNetworkAccessManager(const QByteArray &mockData, QObject *parent)
    : QNetworkAccessManager{parent}, m_mockData(mockData)
{

}

QNetworkReply *MockNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    if (op == QNetworkAccessManager::GetOperation)
    {
        // If it's a GET operation, return a mock reply
        return new MockNetworkReply(m_mockData, this);
    }
    // For all other operations, call the base implementation
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}
