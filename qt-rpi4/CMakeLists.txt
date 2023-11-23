cmake_minimum_required(VERSION 3.16)

project(qt-rpi4 VERSION 0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Core)
qt_standard_project_setup()

add_subdirectory(src)
add_subdirectory(core)
add_subdirectory(test)

#qt_add_executable(appqt-rpi4
#    main.cpp
#)

#qt_add_qml_module(appqt-rpi4
#    URI qt-rpi4
#    VERSION 1.0
#    QML_FILES Main.qml
#)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
set_target_properties(app_qt_rpi4 PROPERTIES
#    MACOSX_BUNDLE_GUI_IDENTIFIER com.example.appqt-rpi4
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

#target_link_libraries(appqt-rpi4
#    PRIVATE Qt6::Quick
#)

#include(GNUInstallDirs)
#install(TARGETS appqt-rpi4
#    BUNDLE DESTINATION .
#    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#)
