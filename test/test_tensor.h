#ifndef TEST_TENSOR_H
#define TEST_TENSOR_H

#include <assert.h>
#include "if_test.h"

namespace test
{


class test_tensor: public if_test
{
public:
        test_tensor();

        void    run() const override;
};

}


#endif // TEST_TENSOR_H
