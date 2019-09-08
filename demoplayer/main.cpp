#include <iostream>
#include <QApplication>
#include <QSurfaceFormat>

#include "../corelib/test/testrunner.h"
#include "app.h"

int main(int argc, char *argv[])
{
    test::test_runner const &runner = test::load(argc, argv);
    runner.run_all();

    QApplication qt(argc, argv);
    App app;
    app.show();
    return qt.exec();
}
