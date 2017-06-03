QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = qt

TEMPLATE = app

CONFIG += c++14

SOURCES += src/main.cpp \
    src/camera.cpp \
    src/tensor.cpp \
    src/geometry.cpp \
    src/light.cpp \
    src/scene.cpp \
    src/renderer.cpp \
    src/material.cpp \
    test/test.cpp \
    test/testrunner.cpp \
    test/testtensor.cpp \
    src/resource.cpp \
    src/pathtracer.cpp \
    src/raster.cpp \
    src/frame.cpp \
    src/mainwindow.cpp \
    test/testgeometry.cpp

HEADERS += \
    src/scene.h \
    src/camera.h \
    src/tensor.h \
    src/geometry.h \
    src/light.h \
    src/renderer.h \
    test/test.h \
    src/material.h \
    test/testrunner.h \
    test/testtensor.h \
    src/resource.h \
    src/pathtracer.h \
    src/raster.h \
    src/frame.h \
    src/mainwindow.h \
    test/testgeometry.h

LIBS += -lGLEW
LIBS += -lglfw
LIBS += -lglut
LIBS += -lGL
LIBS += -lGLU

FORMS += \
    src/mainwindow.ui
