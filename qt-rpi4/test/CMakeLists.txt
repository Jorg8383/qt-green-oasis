cmake_minimum_required(VERSION 3.16)
project(rpi4_tests)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Test REQUIRED)

add_executable(rpi4_tests
    test_main.cpp
    calculatortest.h calculatortest.cpp
    stringtest.h stringtest.cpp
)

target_link_libraries(rpi4_tests PRIVATE Qt6::Core Qt6::Test rpi4_core_lib)

