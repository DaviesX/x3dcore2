#include <iostream>

#include "../src/geometry.h"
#include "../src/resource.h"
#include "testgeometry.h"

test::test_geometry::test_geometry() {}

test::test_geometry::~test_geometry() {}

void test::test_geometry::run() const {
    e8::uv_sphere sphere(e8util::vec3({1, -1, -2}), 5.0f, 20);
    e8util::wavefront_obj obj("uvsphere.obj");
    obj.save_geometry(&sphere);
}
