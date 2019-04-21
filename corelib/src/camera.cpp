#include "tensor.h"
#include "camera.h"


e8::if_camera::if_camera(std::string const& name):
        m_name(name)
{
}

e8::if_camera::~if_camera()
{
}

std::string
e8::if_camera::name() const
{
        return m_name;
}

std::type_info const&
e8::if_camera::interface() const
{
        return typeid(if_camera);
}

e8::if_camera::if_camera(obj_id_t id, std::string const& name):
        if_operable_obj<if_camera>(id),
        m_name(name)
{
}



e8::pinhole_camera::pinhole_camera(e8util::vec3 const& t,
                                   e8util::mat44 const& r,
                                   float sensor_size,
                                   float f,
                                   float aspect):
        e8::pinhole_camera("Unknown_PinholeCamera_Name", t, r, sensor_size, f, aspect)

{
}

e8::pinhole_camera::pinhole_camera(pinhole_camera const& rhs):
        if_camera(id(), name()),
        m_znear(rhs.m_znear),
        m_sensor_size(rhs.m_sensor_size),
        m_focal_len(rhs.m_focal_len),
        m_aspect(rhs.m_aspect),
        m_t(rhs.m_t),
        m_r(rhs.m_r),
        m_proj(rhs.m_proj),
        m_forward(rhs.m_forward)
{
}

e8::pinhole_camera::pinhole_camera(std::string const& name,
                                   e8util::vec3 const& t,
                                   e8util::mat44 const& r,
                                   float sensor_size,
                                   float f,
                                   float aspect):
        e8::if_camera(name),
        m_znear(2*f),
        m_sensor_size(sensor_size),
        m_focal_len(f),
        m_aspect(aspect),
        m_t(t),
        m_r(r)
{
        m_proj = e8util::frustum_perspective2(0.5f*sensor_size/f, aspect, 2.0f*f, 1000.0f).projective_transform();
        update_proj_mat();
}

e8::pinhole_camera::~pinhole_camera()
{
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

void
e8::pinhole_camera::update_proj_mat()
{
        /*e8util::mat44 T = e8util::mat44({1,0,0,0,
                                         0,1,0,0,
                                         0,0,1,0,
                                         t(0),t(1),t(2),1});*/
        e8util::mat44 T_inv = e8util::mat44({1,0,0,0,
                                             0,1,0,0,
                                             0,0,1,0,
                                             -m_t(0),-m_t(1),-m_t(2),1});
        m_forward = m_proj * (~m_r) * T_inv;
}

// TODO: transform needs to be more robust (i.e. handle arbitary linear transformations).
e8::pinhole_camera*
e8::pinhole_camera::transform(e8util::mat44 const& trans) const
{
        pinhole_camera* new_cam = new pinhole_camera(*this);
        new_cam->m_t = e8util::vec3 {trans(0,3), trans(1,3), trans(2,3)};
        new_cam->m_r = e8util::mat44 {trans(0,0), trans(1,0), trans(2,0), 0.0f,
                                      trans(0,1), trans(1,1), trans(2,1), 0.0f,
                                      trans(0,2), trans(1,2), trans(2,2), 0.0f,
                                      0.0f, 0.0f, 0.0f, 1.0f};
        new_cam->update_proj_mat();
        return new_cam;
}
