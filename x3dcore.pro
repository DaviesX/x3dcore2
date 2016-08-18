#-------------------------------------------------
#
# Project created by QtCreator 2016-08-18T00:25:46
#
#-------------------------------------------------

QT       -= core gui

TARGET = x3dcore
TEMPLATE = lib

DEFINES += X3DCORE_LIBRARY

SOURCES += x3dcore.cpp

HEADERS +=\
    include/x3dcore.h \
    include/x3dcore_global.h

unix {
    target.path = /usr/lib
    INSTALLS += target
}

INCLUDEPATH += $$PWD/include
