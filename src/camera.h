#ifndef CAMERA_H
#define CAMERA_H


#include <vector>
#include "tensor.h"


namespace e8
{

class if_camera
{
public:
        if_camera();
        ~if_camera();

        virtual e8util::ray             sample(e8util::rng& rng, unsigned x, unsigned y, unsigned w, unsigned h, float& pdf) const = 0;
        virtual e8util::mat44           projection() const = 0;
};

class pinhole_camera: public if_camera
{
public:
        pinhole_camera(e8util::vec3 const& t, e8util::mat44 const& r, float sensor_size, float f, float aspect);

        e8util::ray                     sample(e8util::rng& rng, unsigned x, unsigned y, unsigned w, unsigned h, float& pdf) const override;
        e8util::mat44                   projection() const override;
private:
        float           m_znear;
        e8util::vec3    m_t;
        e8util::mat44   m_r;
        e8util::mat44   m_proj;
        e8util::mat44   m_forward;
        e8util::mat44   m_inv;
};

}

#endif // CAMERA_H
