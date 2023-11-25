#include <QTest>
#include "configmanagertest.h"
#include "weatherdatatest.h"

int main(int argc, char** argv)
{

    // Use a lambda function to avoid writing the same code for each test suite
    int status = 0;
    auto ASSERT_TEST = [&status, argc, argv](QObject* obj) {
        status |= QTest::qExec(obj, argc, argv);
        delete obj;
    };

    // Add new tests here...
    ASSERT_TEST(new ConfigManagerTest());
    ASSERT_TEST(new WeatherDataTest());

    return status;
}
