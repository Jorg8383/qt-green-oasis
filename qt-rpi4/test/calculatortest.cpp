#include <QTest>
#include "calculatortest.h"

CalculatorTest::CalculatorTest(QObject *parent)
    : QObject{parent}
{

}

void CalculatorTest::testAddition()
{
    Calculator calc;
    QCOMPARE(calc.add(2, 3), 5);
}

void CalculatorTest::testSubstraction()
{
    Calculator calc;
    QCOMPARE(calc.sub(11, 3), 8);
}
