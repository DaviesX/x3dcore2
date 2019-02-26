#ifndef TEST_DIRECT_RENDERER_H
#define TEST_DIRECT_RENDERER_H

#include "test.h"

namespace test
{

class test_direct_renderer: public if_test
{
public:
        test_direct_renderer();
        ~test_direct_renderer();

        void    run() const;
};

}


#endif // TEST_DIRECT_RENDERER_H
