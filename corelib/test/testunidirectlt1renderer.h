#ifndef TESTUNIDIRECTLT1RENDERER_H
#define TESTUNIDIRECTLT1RENDERER_H

#include "test.h"

namespace test {

class test_unidirect_lt1_renderer : public if_test {
  public:
    test_unidirect_lt1_renderer();
    ~test_unidirect_lt1_renderer() override;

    void run() const override;
};

} // namespace test

#endif // TESTUNIDIRECTLT1RENDERER_H
