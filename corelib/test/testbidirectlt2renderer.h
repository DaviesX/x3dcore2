#ifndef TESTBIDIRECTRENDERER_H
#define TESTBIDIRECTRENDERER_H

#include "test.h"

namespace test {

class test_bidirect_lt2_renderer : public if_test {
  public:
    test_bidirect_lt2_renderer();
    ~test_bidirect_lt2_renderer() override;

    void run() const override;
};

} // namespace test

#endif // TESTBIDIRECTRENDERER_H
