#-------------------------------------------------
#
# Project created by QtCreator 2022-09-04T19:13:14
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets  serialport

TARGET = trackingboard
TEMPLATE = app
QT += network



unix|win32: LIBS += -L$$PWD/../../opencv/opencv-3.4.0/build/install/x64/mingw/lib/ -llibopencv_world340.dll

INCLUDEPATH += $$PWD/../../opencv/opencv-3.4.0/build/install/include
DEPENDPATH += $$PWD/../../opencv/opencv-3.4.0/build/install/include

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

win32 {····
INCLUDEPATH += $$PWD/ffmpeg-5.0.1-full_build-shared/include
LIBS += $$PWD/ffmpeg-5.0.1-full_build-shared/lib/avformat.lib   \
        $$PWD/ffmpeg-5.0.1-full_build-shared/lib/avcodec.lib    \
        $$PWD/ffmpeg-5.0.1-full_build-shared/lib/avdevice.lib   \
        $$PWD/ffmpeg-5.0.1-full_build-shared/lib/avfilter.lib   \
        $$PWD/ffmpeg-5.0.1-full_build-shared/lib/avutil.lib     \
        $$PWD/ffmpeg-5.0.1-full_build-shared/lib/postproc.lib   \
        $$PWD/ffmpeg-5.0.1-full_build-shared/lib/swresample.lib \
        $$PWD/ffmpeg-5.0.1-full_build-shared/lib/swscale.lib
#        -L"D:\\ffmpeg-5.0.1-full_build-shared\bin"  -lavcodec-59 -lavdevice-59 -lavfilter-8 -lavformat-59 -lavutil-57 -lpostproc-56 -lswresample-4 -lswscale-6
}
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lavcodec-59
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lavdevice-59
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lavfilter-8
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lavformat-59
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lavutil-57
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lpostproc-56
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lswresample-4
LIBS += -L$$PWD../../ffmpeg-5.0.1-full_build-shared/bin -lswscale-6

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

CONFIG += c++11

INCLUDEPATH += include/

SOURCES += \
    src/main.cpp \
    src/mainwindow.cpp \
    src/camprotocol.cpp \
    src/mythread.cpp \
    src/h264_decoder.cpp \
#    src/rtspclient.cpp \
#    src/mediasession.cpp
#    src/ffmpeg.cpp

HEADERS += \
    include/mainwindow.h \
    include/camprotocol.h \
    include/mythread.h \
    include/h264_decoder.h \
#    include/rtspclient.h \
#    include/mediasession.h
#    include/ffmpeg.h

FORMS += \
    ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
