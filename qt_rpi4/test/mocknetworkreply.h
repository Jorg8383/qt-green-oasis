#ifndef MOCKNETWORKREPLY_H
#define MOCKNETWORKREPLY_H

#include <QtTest>
#include <QNetworkReply>

// This class is used for mocking the network reply when testing the WeatherFetcher class
class MockNetworkReply : public QNetworkReply
{
    Q_OBJECT

public:
    MockNetworkReply(const QByteArray &data, QObject *parent = nullptr);

};

#endif // MOCKNETWORKREPLY_H
