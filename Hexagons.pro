TEMPLATE = app

QT += qml quick multimedia
CONFIG += c++11

SOURCES += main.cpp \
    ioboardfile.cpp

RESOURCES += qml.qrc

# Additional import path used to resolve QML modules in Qt Creator's code model
QML_IMPORT_PATH =

# Default rules for deployment.
include(deployment.pri)

HEADERS += \
    ioboardfile.h

!android{
    LIBS += -L/home/chili/bullet3-2.83.5/build_linux/install/lib
    LIBS += -L/home/chili/ARToolKit5-bin-5.3.2r1-Linux-x86_64/lib
    LIBS+= -lAR -lARICP -lARMulti -lAR2
}
android{
LIBS+= -L/home/chili/ARToolKit5-bin-5.3.2-Android/android/libs/armeabi-v7a/
}


contains(ANDROID_TARGET_ARCH,armeabi-v7a) {
    ANDROID_EXTRA_LIBS = \
        /home/chili/ARToolKit5-bin-5.3.2-Android/android/libs/armeabi-v7a/libc++_shared.so\
        /home/chili/ARToolKit5-bin-5.3.2-Android/android/libs/armeabi-v7a/libARWrapper.so\
}

DISTFILES += \
    android/AndroidManifest.xml \
    android/gradle/wrapper/gradle-wrapper.jar \
    android/gradlew \
    android/res/values/libs.xml \
    android/build.gradle \
    android/gradle/wrapper/gradle-wrapper.properties \
    android/gradlew.bat

ANDROID_PACKAGE_SOURCE_DIR = $$PWD/android
