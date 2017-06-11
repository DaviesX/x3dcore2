#ifndef TEST_RENDERER_H
#define TEST_RENDERER_H

#include "test.h"

namespace test
{

class test_renderer: public if_test
{
public:
        test_renderer();
        ~test_renderer();

        void    run() const;
};

}


#endif // TEST_RENDERER_H
