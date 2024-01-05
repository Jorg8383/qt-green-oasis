#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QNetworkAccessManager>
#include <QQmlContext>
#include <QDebug>
#include <weatherdata.h>
#include <weathermodel.h>
#include <configmanager.h>
#include <weatherfetcher.h>

int main(int argc, char *argv[])
{
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
    // QStringList importPaths = engine.importPathList();
    // qDebug() << "Import Paths:";
    // for (const QString &path : importPaths) {
    //     qDebug() << path;
    // }

    // Initialise the ConfigManager
    try {
        // TODO get rid of absolute path
        ConfigManager::instance().initialise("/home/parallels/QtProjects/qt-green-oasis/qt_rpi4/resources/config/config.ini");
    } catch (const std::exception &e) {
        qDebug() << "Exception ocurred while initialising the ConfigManager: " << e.what();
    }

    // Get the openweather API key from the config.ini file
    auto apiKey = ConfigManager::instance().getValue("Weather/OpenWeatherApiKey");

    // Create objects related to the weather feature
    WeatherModel weatherModel(&app);
    QNetworkAccessManager nam(&app);
    WeatherFetcher weatherFetcher(&nam, weatherModel, apiKey.toString(), &app);

    // For now hardcode the location of the city Ulm
    const double LATITUDE = 48.400002;
    const double LONGITUDE = 9.983333;


    return app.exec();
}
