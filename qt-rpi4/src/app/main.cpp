#include <QGuiApplication>
#include <QQmlApplicationEngine>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    QQmlApplicationEngine engine;
    // Follow the new URL policy introduced in Qt6.5, where ':/qt/qml/' is the default resource prefix for QML modules.
    const QUrl url(u"qrc:/qt/qml/qt-rpi4/Main.qml"_qs);

/* This commented out URL below follows the previous policy used in version prior to Qt6.5
 * see app/CMakeLists.txt -> qt_policy(SET QTP0001 NEW)
 * https://doc.qt.io/qt-6/qt-cmake-policy-qtp0001.html */
//    const QUrl url(u"qrc:/qt-rpi4/Main.qml"_qs);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed,
        &app, []() { QCoreApplication::exit(-1); },
        Qt::QueuedConnection);
    engine.load(url);

    return app.exec();
}
