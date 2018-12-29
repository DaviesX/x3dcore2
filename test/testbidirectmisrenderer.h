#ifndef TESTBIDIRECTRENDERER_H
#define TESTBIDIRECTRENDERER_H

#include "test.h"

namespace test
{

class test_bidirect_mis_renderer: public if_test
{
public:
        test_bidirect_mis_renderer();
        ~test_bidirect_mis_renderer() override;

        void    run() const override;
};

}


#endif // TESTBIDIRECTRENDERER_H
