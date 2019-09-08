#ifndef TEST_TENSOR_H
#define TEST_TENSOR_H

#include "test.h"
#include <assert.h>

namespace test {

class test_tensor : public if_test
{
public:
    test_tensor();

    void run() const override;
};

} // namespace test

#endif // TEST_TENSOR_H
