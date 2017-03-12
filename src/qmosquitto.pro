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

header_files.files = qmosquitto.h qmosquitto_global.h

unix {
    header_files.path = /usr/include
    target.path = /usr/lib
    INSTALLS += target
    INSTALLS += header_files
}

release:DESTDIR = release
release:OBJECTS_DIR = release/.obj
release:MOC_DIR = release/.moc
release:RCC_DIR = release/.rcc
release:UI_DIR = release/.ui

debug:DESTDIR = debug
debug:OBJECTS_DIR = debug/.obj
debug:MOC_DIR = debug/.moc
debug:RCC_DIR = debug/.rcc
debug:UI_DIR = debug/.ui
