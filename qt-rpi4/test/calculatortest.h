#ifndef CALCULATORTEST_H
#define CALCULATORTEST_H

#include <QObject>
#include <calculator.h>

class CalculatorTest : public QObject
{
    Q_OBJECT
public:
    explicit CalculatorTest(QObject *parent = nullptr);

signals:

private slots:
    void testAddition();
    void testSubstraction();
};

#endif // CALCULATORTEST_H
