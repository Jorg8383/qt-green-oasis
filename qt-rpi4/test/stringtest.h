#ifndef STRINGTEST_H
#define STRINGTEST_H

#include <QObject>
#include <QTest>
#include <stringgenerator.h>

class StringTest : public QObject
{
    Q_OBJECT
public:
    explicit StringTest(QObject *parent = nullptr);

signals:

private slots:
    void testWelcomeMessage();
};

#endif // STRINGTEST_H
