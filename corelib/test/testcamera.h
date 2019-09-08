#ifndef TEST_CAMERA_H
#define TEST_CAMERA_H

#include "test.h"

namespace test {

class test_camera : public if_test
{
public:
    test_camera();
    ~test_camera();

    void run() const;
};

} // namespace test

#endif // TEST_CAMERA_H
