#-------------------------------------------------
#
# e8yescg corelib
#
#-------------------------------------------------

QT       += core opengl
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
TARGET = e8yescg

TEMPLATE = lib

QMAKE_CXXFLAGS_RELEASE -= -O2
QMAKE_CXXFLAGS_RELEASE += -Ofast -march=native
QMAKE_LFLAGS_RELEASE += -Ofast -march=native

CONFIG += c++17

SOURCES += \
    src/camera.cpp \
    src/tensor.cpp \
    src/geometry.cpp \
    src/light.cpp \
    src/renderer.cpp \
    src/material.cpp \
    src/util.cpp \
    test/test.cpp \
    test/testrunner.cpp \
    test/testtensor.cpp \
    src/resource.cpp \
    src/pathtracer.cpp \
    src/raster.cpp \
    src/frame.cpp \
    src/pathtracerfact.cpp \
    src/pipeline.cpp \
    src/thread.cpp \
    src/compositor.cpp \
    test/testgeometry.cpp \
    test/testresource.cpp \
    test/testcamera.cpp \
    test/testframe.cpp \
    test/testscene.cpp \
    test/testdirectrenderer.cpp \
    test/testunidirectrenderer.cpp \
    test/testbidirectmisrenderer.cpp \
    test/testbidirectlt2renderer.cpp \
    test/testgltfresource.cpp \
    src/objdb.cpp \
    src/obj.cpp \
    src/pathspace.cpp \
    src/cinematics.cpp \
    test/testobjdb.cpp \
    src/lightsources.cpp


HEADERS += \
    src/camera.h \
    src/tensor.h \
    src/geometry.h \
    src/light.h \
    src/renderer.h \
    src/util.h \
    test/test.h \
    src/material.h \
    test/testrunner.h \
    test/testtensor.h \
    src/resource.h \
    src/pathtracer.h \
    src/raster.h \
    src/frame.h \
    src/thread.h \
    src/compositor.h \
    src/pathtracerfact.h \
    src/pipeline.h \
    test/testgeometry.h \
    test/testresource.h \
    test/testcamera.h \
    test/testframe.h \
    test/testscene.h \
    test/testdirectrenderer.h \
    test/testunidirectrenderer.h \
    test/testbidirectmisrenderer.h \
    test/testbidirectlt2renderer.h \
    test/testgltfresource.h \
    src/objdb.h \
    src/obj.h \
    src/pathspace.h \
    src/cinematics.h \
    test/testobjdb.h \
    src/lightsources.h

win32 {
LIBS += -lOpengl32
}
