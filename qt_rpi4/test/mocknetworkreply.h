#ifndef MOCKNETWORKREPLY_H
#define MOCKNETWORKREPLY_H

#include <QtTest>
#include <QSignalSpy>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// This class is used for mocking the network reply when testing the WeatherFetcher class
class MockNetworkReply : public QNetworkReply
{
public:
    MockNetworkReply(const QByteArray &data, QObject *parent = nullptr);

};

#endif // MOCKNETWORKREPLY_H
