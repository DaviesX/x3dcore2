#include <iostream>
#include <assert.h>
#include "src/camera.h"
#include "testcamera.h"


test::test_camera::test_camera()
{
}

test::test_camera::~test_camera()
{
}

void
test::test_camera::run() const
{
        e8util::vec3 const& trans = e8util::vec3({1.0f, 2.0f, 3.0f});
        e8util::mat44 const& rot = e8util::mat44_rotate(M_PI/4, e8util::vec3({0.0f, 1.0f, 0.0f}));
        float const sensor_size = 0.032f;
        float const focal_len = 0.035f;
        float const aspect = 4.0f/3.0f;

        e8::pinhole_camera cam(trans, rot, sensor_size, focal_len, aspect);

        unsigned const resx = 1025;
        unsigned const resy = 769;
        e8util::rng rng(10);

        float s;
        e8util::ray const& r = cam.sample(rng, (resx - 1)/2, (resy - 1)/2, resx, resy, s);
        assert(r.v() == e8util::vec3({-1.0f/std::sqrt(2), 0.0f, -1.0f/std::sqrt(2)}));

        for (unsigned j = 0; j < resy; j ++) {
                for (unsigned i = 0; i < resx; i ++) {
                        float pdf;
                        e8util::ray const& ray = cam.sample(rng, i, j, resx, resy, pdf);
                        assert(ray.o() == trans);

                        float x = (static_cast<float>(i)/(resx - 1)*sensor_size - sensor_size/2.0f)*aspect;
                        float y = (resy - 1 - static_cast<float>(j))/(resy - 1)*sensor_size - sensor_size/2.0f;
                        float z = -focal_len;
                        e8util::vec3 v({x, y, z});
                        v = v.normalize();
                        v = (rot*v.homo(0.0f)).trunc();

                        assert(ray.v() == v);
                }
        }
}
