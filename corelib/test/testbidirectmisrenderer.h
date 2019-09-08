#ifndef TESTBIDIRECTMISRENDERER_H
#define TESTBIDIRECTMISRENDERER_H

#include "test.h"

namespace test {

class test_bidirect_mis_renderer : public if_test
{
public:
    test_bidirect_mis_renderer();
    ~test_bidirect_mis_renderer() override;

    void run() const override;
};

} // namespace test

#endif // TESTBIDIRECTMISRENDERER_H
