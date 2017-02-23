#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <iostream>
#include <QtQml>
#include <stdlib.h>
#include <ioboardfile.h>


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    qmlRegisterType<IOBoardFile>("IOBoardFile", 1, 0, "IOBoardFile");
    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/main.qml")));
#ifdef ANDROID
    engine.rootContext()->setContextProperty("Platform","ANDROID");
    QString conf_folder="file:"+QString(getenv("EXTERNAL_STORAGE"))+"/staTIc/";
    engine.rootContext()->setContextProperty("Conf_folder",conf_folder);
#else
    engine.rootContext()->setContextProperty("Platform","UNIX");
    QString conf_folder="file:"+QString(getenv("HOME"));
    engine.rootContext()->setContextProperty("Conf_folder",conf_folder);
#endif
    return app.exec();
}
