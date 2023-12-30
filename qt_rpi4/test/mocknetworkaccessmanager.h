#ifndef MOCKNETWORKACCESSMANAGER_H
#define MOCKNETWORKACCESSMANAGER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

class MockNetworkAccessManager : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit MockNetworkAccessManager(const QByteArray &mockData, QObject *parent = nullptr);

protected:
    QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                 QIODevice *outgoingData = nullptr) override;

signals:

private:
    QByteArray m_mockData;
};

#endif // MOCKNETWORKACCESSMANAGER_H
