cmake_minimum_required(VERSION 3.16)
project(rpi4_core_lib)

set(CMAKE_AUTOMOC ON)

find_package(Qt6 COMPONENTS Core REQUIRED)

add_library(rpi4_core_lib STATIC


    configmanager.h configmanager.cpp
    logger.h logger.cpp
)

target_link_libraries(rpi4_core_lib PRIVATE Qt6::Core)

# This line tells CMake to add the directory of the src/core/CMakeLists.txt file
# to the include path when compiling the rpi4_core_lib target and any targets that
# link to rpi4_core_lib. It enables to include header files of the core library
# in other targets (test, app, etc.) like this:
# #include <headerfile.h>
# instead of:
# #include "../src/core/headerfile.h"
target_include_directories(rpi4_core_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
