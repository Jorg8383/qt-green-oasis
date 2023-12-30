#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <weatherdata.h>
#include <weathermodel.h>
#include <configmanager.h>

int main(int argc, char *argv[])
{
    // Initialise the ConfigManager
    try {
        // TODO get rid of absolute path
        ConfigManager::instance().initialise("/home/parallels/QtProjects/qt-green-oasis/qt_rpi4/resources/config/config.ini");
    } catch (const std::exception &e) {
        qDebug() << "Exception ocurred while initialising the ConfigManager: " << e.what();
    }

    QGuiApplication app(argc, argv);

    // Register and expose C++ classes to QML
    qmlRegisterType<WeatherData>("com.greenoasis.weatherdata", 1, 0, "WeatherData");
    qmlRegisterType<WeatherModel>("com.greenoasis.weathermodel", 1, 0, "WeatherModel");

    QQmlApplicationEngine engine;

    // Follow the new URL policy introduced in Qt6.5, where ':/qt/qml/' is the default resource prefix for QML modules.
    const QUrl url(u"qrc:/qt/qml/qt_rpi4/qml/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    // Print the Qt import paths list to the console for debugging purposes
    QStringList importPaths = engine.importPathList();
    qDebug() << "Import Paths:";
    for (const QString &path : importPaths) {
        qDebug() << path;
    }

    return app.exec();
}
