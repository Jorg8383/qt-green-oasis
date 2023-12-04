cmake_minimum_required(VERSION 3.16)
project(rpi4_weather_lib)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Network)


add_library(rpi4_weather_lib STATIC
    weatherdata.h weatherdata.cpp
    weathermodel.h weathermodel.cpp
    weatherfetcher.h weatherfetcher.cpp


#    weatherdata.h weatherdata.cpp
)

target_link_libraries(rpi4_weather_lib PRIVATE Qt6::Core Qt6::Network)

# This line tells CMake to add the directory of the src/weather/CMakeLists.txt file
# to the include path when compiling the rpi4_core_lib target and any targets that
# link to rpi4_weather_lib. It enables to include header files of the core library
# in other targets (test, app, etc.) like this:
# #include <headerfile.h>
# instead of:
# #include "../src/weather/headerfile.h"
target_include_directories(rpi4_weather_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
