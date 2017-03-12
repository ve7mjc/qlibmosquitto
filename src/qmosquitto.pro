#-------------------------------------------------
#
# Project created by QtCreator 2017-02-20T19:25:54
#
#-------------------------------------------------

QT       -= gui

TARGET = qmosquitto
TEMPLATE = lib

LIBS += -lmosquitto -lmosquittopp

DEFINES += QLIBMOSQUITTO_LIBRARY

SOURCES += qmosquitto.cpp

HEADERS += qmosquitto.h\
        qmosquitto_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}
