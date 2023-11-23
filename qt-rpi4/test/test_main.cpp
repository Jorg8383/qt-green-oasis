#include <QTest>
#include "calculatortest.h"
#include "stringtest.h"

int main(int argc, char** argv)
{

    // Use a lambda function to avoid writing the same code for each test class
    int status = 0;
    auto ASSERT_TEST = [&status, argc, argv](QObject* obj) {
        status |= QTest::qExec(obj, argc, argv);
        delete obj;
    };

    // Add new tests here...
    ASSERT_TEST(new CalculatorTest());
    ASSERT_TEST(new StringTest());

    return status;
}
