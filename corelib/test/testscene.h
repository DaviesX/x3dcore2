#ifndef TEST_SCENE_H
#define TEST_SCENE_H

#include "test.h"

namespace test {

class test_path_space : public if_test {
  public:
    test_path_space();
    ~test_path_space() override;

    void run() const override;
};

} // namespace test

#endif // TEST_SCENE_H
