#ifndef CONFIGMANAGERTEST_H
#define CONFIGMANAGERTEST_H

#include <QObject>
#include <QTest>
#include <configmanager.h>

class ConfigManagerTest : public QObject
{
    Q_OBJECT
public:
    explicit ConfigManagerTest(QObject *parent = nullptr);

signals:

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetValue();
    void testKeyNotFound();

};

#endif // CONFIGMANAGERTEST_H
