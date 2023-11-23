#include "calculator.h"

Calculator::Calculator(QObject *parent)
    : QObject{parent}
{

}

int Calculator::add(int a, int b)
{
    return a + b;
}
