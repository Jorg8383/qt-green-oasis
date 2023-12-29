#include "mocknetworkaccessmanager.h"
#include "mocknetworkreply.h"

MockNetworkAccessManager::MockNetworkAccessManager(const QByteArray &mockData, QObject *parent)
    : QNetworkAccessManager{parent}, m_mockData(mockData)
{

}

QNetworkReply *MockNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request, QIODevice *outgoingData)
{
    Q_UNUSED(op);
    Q_UNUSED(request);
    Q_UNUSED(outgoingData);
    return new MockNetworkReply(m_mockData, this);
}
