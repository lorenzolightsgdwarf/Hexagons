TEMPLATE = app

QT += qml quick multimedia
CONFIG += c++11

SOURCES += main.cpp \
    imageanayser.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    imageanayser.h

LIBS+= -L/home/chili/artoolkit5/lib -L/home/chili/artoolkit5/lib/linux-x86_64

android{
LIBS+= -L/home/chili/artoolkit5/android/libs/armeabi-v7a/
}


contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        /home/chili/artoolkit5/android/libs/armeabi-v7a/libc++_shared.so\
        /home/chili/artoolkit5/android/libs/armeabi-v7a/libARWrapper.so
}
