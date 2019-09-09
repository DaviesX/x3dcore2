#ifndef TEST_GEOMETRY_H
#define TEST_GEOMETRY_H

#include "test.h"

namespace test {

class test_geometry : public if_test {
  public:
    test_geometry();
    ~test_geometry() override;

    void run() const override;
};

} // namespace test

#endif // TEST_GEOMETRY_H
