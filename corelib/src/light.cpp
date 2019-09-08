#include "light.h"
#include "tensor.h"
#include <cmath>

e8::if_light::if_light(std::string const &name) : m_name(name) {}

e8::if_light::~if_light() {}

void e8::if_light::set_scene_boundary(e8util::aabb const & /* bbox */) {}

std::string e8::if_light::name() const
{
    return m_name;
}

e8::obj_protocol e8::if_light::protocol() const
{
    return obj_protocol::obj_protocol_light;
}

e8::if_light::if_light(obj_id_t id, std::string const &name)
    : if_operable_obj<if_light>(id), m_name(name)
{}

e8::area_light::area_light(std::string const &name, if_geometry const &geo, e8util::vec3 const &rad)
    : if_light(name), m_geo(geo), m_rad(rad),
      m_power(static_cast<float>(M_PI) * m_geo.surface_area() * rad)
{}

e8::area_light::area_light(if_geometry const &geo, e8util::vec3 const &rad)
    : area_light("Unkown_Area_Light_Name", geo, rad)
{}

e8::area_light::area_light(area_light const &other)
    : if_light(other.id(), other.name()), m_geo(other.m_geo), m_rad(other.m_rad),
      m_power(other.m_power)
{}

void e8::area_light::sample(e8util::rng &rng,
                            float &p_pdf,
                            float &w_pdf,
                            e8util::vec3 &p,
                            e8util::vec3 &n,
                            e8util::vec3 &w) const
{
    m_geo.sample(rng, p, n, p_pdf);
    w = e8util::vec3_cos_hemisphere_sample(n, rng.draw(), rng.draw());
    w_pdf = n.inner(w) / static_cast<float>(M_PI);
}

void e8::area_light::sample(e8util::rng &rng, float &pdf, e8util::vec3 &p, e8util::vec3 &n) const
{
    m_geo.sample(rng, p, n, pdf);
}

e8util::vec3 e8::area_light::eval(e8util::vec3 const &i, e8util::vec3 const &n) const
{
    float r2 = i.inner(i);
    float cos = (i / std::sqrt(r2)).inner(n);
    if (cos > 0)
        return cos / r2 * m_rad;
    else
        return 0.0f;
}

e8util::vec3 e8::area_light::emission(e8util::vec3 const &w, e8util::vec3 const &n) const
{
    float cos = w.inner(n);
    if (cos > 0)
        return m_rad * cos;
    else
        return 0.0f;
}

e8util::vec3 e8::area_light::power() const
{
    return m_power;
}

std::unique_ptr<e8::if_light> e8::area_light::copy() const
{
    return std::make_unique<area_light>(*this);
}

std::unique_ptr<e8::if_light> e8::area_light::transform(e8util::mat44 const & /* trans */) const
{
    return copy();
}

e8::sky_light::sky_light(std::string const &name, e8util::vec3 const &rad)
    : if_light(name), m_rad(rad)
{}

e8::sky_light::sky_light(e8util::vec3 const &rad) : sky_light("Unkown_Sky_Light_Name", rad) {}

e8::sky_light::sky_light(sky_light const &other)
    : if_light(other.id(), other.name()), m_rad(other.m_rad), m_ref_p(other.m_ref_p),
      m_dia(other.m_dia)
{}

void e8::sky_light::set_scene_boundary(e8util::aabb const &bbox)
{
    m_dia = 2 * bbox.enclosing_radius();
    m_ref_p = bbox.centroid();
    m_ref_p(2) = 0.0f;
}

void e8::sky_light::sample(e8util::rng &rng,
                           float &p_pdf,
                           float &w_pdf,
                           e8util::vec3 &p,
                           e8util::vec3 &n,
                           e8util::vec3 &w) const
{
    e8util::vec3 z({0, 0, 1});
    e8util::vec3 const &u = e8util::vec3_cos_hemisphere_sample(z, rng.draw(), rng.draw());
    w = -u;
    n = -u;
    p = (u + m_ref_p) * m_dia;

    p_pdf = z.inner(u) / static_cast<float>(M_PI);
    w_pdf = 1.0f;
}

void e8::sky_light::sample(e8util::rng &rng, float &pdf, e8util::vec3 &p, e8util::vec3 &n) const
{
    e8util::vec3 z({0, 0, 1});
    e8util::vec3 const &u = e8util::vec3_cos_hemisphere_sample(z, rng.draw(), rng.draw());
    n = -u;
    p = (u + m_ref_p) * m_dia;

    pdf = z.inner(u) / static_cast<float>(M_PI);
}

e8util::vec3 e8::sky_light::eval(e8util::vec3 const &i, e8util::vec3 const &n) const
{
    float cos = i.normalize().inner(n);
    return m_rad * std::max(cos, 0.0f);
}

e8util::vec3 e8::sky_light::emission(e8util::vec3 const &w, e8util::vec3 const &n) const
{
    return m_rad * std::max(w.inner(n), 0.0f);
}

e8util::vec3 e8::sky_light::power() const
{
    return static_cast<float>(M_PI) * (m_dia * m_dia / 2);
}

std::unique_ptr<e8::if_light> e8::sky_light::copy() const
{
    return std::make_unique<sky_light>(*this);
}

std::unique_ptr<e8::if_light> e8::sky_light::transform(e8util::mat44 const & /* trans */) const
{
    return copy();
}
