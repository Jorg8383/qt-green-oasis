#include <QtTest>
#include "../core/calculator.h"

class TestCalculator : public QObject
{
    Q_OBJECT

private slots:
    void testAddition()
    {
        Calculator calc;
        QCOMPARE(calc.add(2, 3), 5);
    }
};

QTEST_APPLESS_MAIN(TestCalculator)
#include "TestCalculator.moc"
