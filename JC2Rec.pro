#-------------------------------------------------
#
# Project created by QtCreator 2015-05-23T17:32:54
#
#-------------------------------------------------

QT       += multimedia core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JC2Rec
TEMPLATE = app


INCLUDEPATH += ../codec2/src
INCLUDEPATH += ../codec2/build/codec2


SOURCES += main.cpp\
        mainwindow.cpp \
    source.cpp \
    sink.cpp

HEADERS  += mainwindow.h \
        ../codec2/src/codec2.h \
    source.h \
    sink.h

FORMS    += mainwindow.ui

win32 {
LIBS += -L../codec2/build/src \
    -lcodec2
}

RESOURCES += \
    icon.qrc
