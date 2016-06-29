#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "imageanayser.h"
#include <QtQml>
#include "filterfactory.h"
#include <stdlib.h>
#define MARKER_LEN 35.f
#define MARKER_DIST 90.f
#define MARKER_ORIGIN 0
#define MARKER_ELEV 0.f

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qmlRegisterType<FilterFactory>("FilterFactory", 1, 0, "FilterFactory");
    qmlRegisterType<ImageAnayser>("ImageAnayser", 1, 0, "ImageAnayser");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));

    return app.exec();
}
