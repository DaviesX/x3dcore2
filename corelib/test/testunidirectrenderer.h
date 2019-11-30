#ifndef TESTUNIDIRECTRENDERER_H
#define TESTUNIDIRECTRENDERER_H

#include "test.h"

namespace test {

class test_unidirect_renderer : public if_test {
  public:
    test_unidirect_renderer() = default;
    ~test_unidirect_renderer() override = default;

    void run() const override;
};

} // namespace test

#endif // TESTUNIDIRECTRENDERER_H
