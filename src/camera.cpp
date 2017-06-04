#include "tensor.h"
#include "camera.h"


e8::if_camera::if_camera()
{
}

e8::if_camera::~if_camera()
{
}



e8::pinhole_camera::pinhole_camera(e8util::vec3 const& t, e8util::mat44 const& r, float sensor_size, float f, float aspect):
        m_znear(2*f), m_t(t), m_r(r)
{
        m_proj = e8util::frustum_perspective2(0.5f*sensor_size/f, aspect, 2.0f*f, 1000.0f).projective_transform();

        e8util::mat44 T = e8util::mat44({1,0,0,0,
                                         0,1,0,0,
                                         0,0,1,0,
                                         t(0),t(1),t(2),1});
        e8util::mat44 T_inv = e8util::mat44({1,0,0,0,
                                             0,1,0,0,
                                             0,0,1,0,
                                             -t(0),-t(1),-t(2),1});
        m_forward = m_proj * (~m_r) * T_inv;
        m_inv = T * m_r * m_proj ^ (-1);
}

std::vector<e8util::ray>
e8::pinhole_camera::sample(unsigned x, unsigned y, unsigned w, unsigned h, unsigned) const
{
        e8util::vec4 const& e = m_inv*e8util::vec4({-m_znear*static_cast<float>(x)/w, -m_znear*static_cast<float>(y)/h, -m_znear, m_znear});
        e8util::vec3 const& v = (e.trunc() - m_t).normalize();
        e8util::ray ray(m_t, v);
        return std::vector<e8util::ray>({ray});
}

e8util::mat44
e8::pinhole_camera::projection() const
{
        return m_forward;
}
