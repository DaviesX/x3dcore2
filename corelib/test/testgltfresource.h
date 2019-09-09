#ifndef TESTGLTFRESOURCE_H
#define TESTGLTFRESOURCE_H

#include "test/test.h"

namespace test {

class test_gltf_resource : public if_test {
  public:
    test_gltf_resource();
    ~test_gltf_resource() override;

    void run() const override;
};

} // namespace test

#endif // TESTGLTFRESOURCE_H
