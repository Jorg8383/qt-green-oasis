#include "stringtest.h"

StringTest::StringTest(QObject *parent)
    : QObject{parent}
{

}

void StringTest::testWelcomeMessage()
{
    StringGenerator sgen;
    QCOMPARE(sgen.generateWelcomeMessage(), QString("Welcome to Qt!"));
}
