#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QDebug>
#include <weatherdata.h>
#include <weathermodel.h>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    // Register and expose C++ classes to QML
    qmlRegisterType<WeatherData>("com.weather.weatherdata", 1, 0, "WeatherData");
    qmlRegisterType<WeatherModel>("com.weather.weathermodel", 1, 0, "WeatherModel");

    QQmlApplicationEngine engine;
    // Follow the new URL policy introduced in Qt6.5, where ':/qt/qml/' is the default resource prefix for QML modules.
    const QUrl url(u"qrc:/qt/qml/qt_rpi4/Main.qml"_qs);

    QStringList importPaths = engine.importPathList();

    qDebug() << "Import Paths:";
    for (const QString &path : importPaths) {
        qDebug() << path;
    }

/* This commented out URL below follows the previous policy used in version prior to Qt6.5
 * see app/CMakeLists.txt -> qt_policy(SET QTP0001 NEW)
 * https://doc.qt.io/qt-6/qt-cmake-policy-qtp0001.html */
//    const QUrl url(u"qrc:/qt_rpi4/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
