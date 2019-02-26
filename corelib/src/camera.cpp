#include "tensor.h"
#include "camera.h"


e8::if_camera::if_camera()
{
}

e8::if_camera::~if_camera()
{
}



e8::pinhole_camera::pinhole_camera(e8util::vec3 const& t, e8util::mat44 const& r, float sensor_size, float f, float aspect):
        m_znear(2*f), m_sensor_size(sensor_size), m_focal_len(f), m_aspect(aspect), m_t(t), m_r(r)
{
        m_proj = e8util::frustum_perspective2(0.5f*sensor_size/f, aspect, 2.0f*f, 1000.0f).projective_transform();

        /*e8util::mat44 T = e8util::mat44({1,0,0,0,
                                         0,1,0,0,
                                         0,0,1,0,
                                         t(0),t(1),t(2),1});*/
        e8util::mat44 T_inv = e8util::mat44({1,0,0,0,
                                             0,1,0,0,
                                             0,0,1,0,
                                             -t(0),-t(1),-t(2),1});
        m_forward = m_proj * (~m_r) * T_inv;
}

e8util::ray
e8::pinhole_camera::sample(e8util::rng&, unsigned x, unsigned y, unsigned w, unsigned h, float& pdf) const
{
        e8util::vec3 v({(static_cast<float>(x)/(w - 1)*m_sensor_size - m_sensor_size/2.0f)*m_aspect,
                        (h - 1 - static_cast<float>(y))/(h - 1)*m_sensor_size - m_sensor_size/2.0f,
                        -m_focal_len});

        e8util::ray ray(m_t, (m_r*v.homo(0.0f)).trunc().normalize());
        pdf = 1.0f;
        return ray;
}

e8util::mat44
e8::pinhole_camera::projection() const
{
        return m_forward;
}
