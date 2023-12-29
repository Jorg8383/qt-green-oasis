cmake_minimum_required(VERSION 3.16)
project(rpi4_tests)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core Test Network REQUIRED)

add_executable(rpi4_tests
    test_main.cpp
    configmanagertest.h configmanagertest.cpp
    weatherdatatest.h weatherdatatest.cpp
    weathermodeltest.h weathermodeltest.cpp
    mocknetworkreply.h mocknetworkreply.cpp
    weatherfetchertest.h weatherfetchertest.cpp
    mocknetworkaccessmanager.h mocknetworkaccessmanager.cpp
)

target_link_libraries(rpi4_tests PRIVATE Qt6::Core Qt6::Test Qt6::Network rpi4_core_lib rpi4_weather_lib)

