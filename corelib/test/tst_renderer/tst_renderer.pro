QT += testlib opengl
QT -= gui

CONFIG += qt console warn_on depend_includepath testcase
CONFIG -= app_bundle
CONFIG += c++17

TEMPLATE = app

SOURCES += \ 
    tst_renderer.cpp

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../../release/ -le8yescg
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../../debug/ -le8yescg
else:unix: LIBS += -L$$OUT_PWD/../../ -le8yescg

INCLUDEPATH += $$PWD/../../
DEPENDPATH += $$PWD/../../
