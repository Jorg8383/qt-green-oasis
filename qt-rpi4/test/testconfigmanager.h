#ifndef TESTCONFIGMANAGER_H
#define TESTCONFIGMANAGER_H

#include <QObject>
#include <QTest>
#include <configmanager.h>

class TestConfigManager : public QObject
{
    Q_OBJECT
public:
    explicit TestConfigManager(QObject *parent = nullptr);

signals:

private slots:
    void initTestCase();
    void cleanupTestCase();
    void testGetValue();

};

#endif // TESTCONFIGMANAGER_H
