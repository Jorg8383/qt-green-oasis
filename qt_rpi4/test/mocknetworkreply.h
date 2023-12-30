#ifndef MOCKNETWORKREPLY_H
#define MOCKNETWORKREPLY_H

#include <QtTest>
#include <QSharedPointer>
#include <QNetworkReply>
#include <QBuffer>

// This class is used for mocking the network reply when testing the WeatherFetcher class

class MockNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:
    MockNetworkReply(const QByteArray &data, QObject *parent = nullptr);
    QSharedPointer<QNetworkReply> createMockNetworkReply(const QByteArray &data);

protected:
    qint64 readData(char *data, qint64 maxlen) override;
    void abort() override;

private slots:
    void networkDelaySimulation();

private:
    QByteArray m_data;
};

#endif // MOCKNETWORKREPLY_H
