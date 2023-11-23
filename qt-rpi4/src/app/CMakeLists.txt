find_package(Qt6 6.4 REQUIRED COMPONENTS Core Quick)

add_executable(app_qt_rpi4
    main.cpp
)

#qt_policy(QTP0001 NEW)

qt_add_qml_module(app_qt_rpi4
    URI qt-rpi4
    VERSION 1.0
    QML_FILES Main.qml
)

target_link_libraries(app_qt_rpi4
    PRIVATE Qt6::Quick rpi4_core_lib
)

include(GNUInstallDirs)
install(TARGETS app_qt_rpi4
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
