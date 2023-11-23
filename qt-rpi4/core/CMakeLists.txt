cmake_minimum_required(VERSION 3.16)
project(rpi4_core_lib)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(rpi4_core_lib STATIC
    calculator.h calculator.cpp
    stringgenerator.h stringgenerator.cpp
)

target_link_libraries(rpi4_core_lib PRIVATE Qt6::Core)

