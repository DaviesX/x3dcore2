#ifndef TEST_SCENE_H
#define TEST_SCENE_H

#include "test.h"

namespace test
{

class test_scene: public if_test
{
public:
        test_scene();
        ~test_scene();

        void    run() const override;
};

}

#endif // TEST_SCENE_H
