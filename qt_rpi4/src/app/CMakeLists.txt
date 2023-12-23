find_package(Qt6 REQUIRED COMPONENTS Core Quick Qml)

add_executable(app_qt_rpi4
    main.cpp
)

# qt_policy(SET QTP0001 NEW)

list(APPEND MODULE_QML_FILES
    qml/Main.qml
    qml/WeatherPage.qml
)
list(APPEND MODULE_JS_FILES

)

qt_add_qml_module(app_qt_rpi4
    URI qt_rpi4
    VERSION 1.0
    QML_FILES ${MODULE_QML_FILES}
    # QML_FILES qml/Main.qml
)

target_link_libraries(app_qt_rpi4
    PRIVATE Qt6::Quick Qt6::Qml rpi4_core_lib rpi4_weather_lib
)

include(GNUInstallDirs)
install(TARGETS app_qt_rpi4
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
