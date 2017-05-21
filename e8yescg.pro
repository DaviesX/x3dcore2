TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.cpp \
    src/if_material.cpp \
    src/if_renderable.cpp \
    src/camera.cpp \
    src/tensor.cpp \
    src/if_scene.cpp \
    src/if_light.cpp \
    src/if_renderer.cpp \
    test/if_test.cpp \
    test/test_tensor.cpp \
    test/test_runner.cpp

HEADERS += \
    src/if_material.h \
    src/if_light.h \
    src/if_renderable.h \
    src/scene.h \
    src/camera.h \
    src/tensor.h \
    src/if_renderer.h \
    test/if_test.h \
    test/test_tensor.h \
    test/test_runner.h
