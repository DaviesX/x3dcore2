#ifndef TESTRESOURCE_H
#define TESTRESOURCE_H

#include "test.h"

namespace test {

class test_resource : public if_test {
  public:
    test_resource();
    ~test_resource() override;

    void run() const override;
};

} // namespace test

#endif // TESTRESOURCE_H
