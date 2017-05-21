#include <iostream>
#include "test/testrunner.h"

int
main(int argc, char *argv[])
{
        test::test_runner const& runner = test::load(argc, argv);
        runner.run_all();
        return 0;
}
