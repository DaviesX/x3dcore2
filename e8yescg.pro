TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

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
    src/frame.cpp

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
    src/frame.h
