#ifndef TEST_FRAME_H
#define TEST_FRAME_H

#include "test.h"

namespace test
{

class test_frame: public if_test
{
public:
        test_frame();
        ~test_frame();

        void    run() const override;
};

}

#endif // TEST_FRAME_H
