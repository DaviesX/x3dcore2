#include <iostream>
#include "test/test_runner.h"

int
main(int argc, char *argv[])
{
        test::test_runner const& runner = test::load(argc, argv);
        runner.run_all();
        return 0;
}
