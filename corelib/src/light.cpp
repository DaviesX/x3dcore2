#include "light.h"
#include "tensor.h"
#include <algorithm>
#include <cmath>

e8::if_light::if_light(std::string const &name) : if_operable_obj<if_light>(name) {}

e8::if_light::~if_light() {}

void e8::if_light::set_scene_boundary(e8util::aabb const & /* bbox */) {}

e8::obj_protocol e8::if_light::protocol() const { return obj_protocol::obj_protocol_light; }

e8::if_light::if_light(obj_id_t id, std::string const &name)
    : if_operable_obj<if_light>(id, name) {}

e8::area_light::area_light(std::string const &name, std::shared_ptr<if_geometry> const &geo,
                           e8util::vec3 const &rad)
    : if_light(name), m_geo(geo), m_rad(rad),
      m_power(static_cast<float>(M_PI) * m_geo->surface_area() * rad) {}

e8::area_light::area_light(area_light const &other)
    : if_light(other.id(), other.name()), m_geo(other.m_geo), m_rad(other.m_rad),
      m_power(other.m_power) {}

e8::if_light::emission_sample e8::area_light::sample_emssion(e8util::rng *rng) const {
    emission_sample sample;
    sample.surface = m_geo->sample(rng);
    sample.w = e8util::vec3_cos_hemisphere_sample(sample.surface.n, rng->draw(), rng->draw());
    sample.solid_angle_dens = sample.surface.n.inner(sample.w) / static_cast<float>(M_PI);
    return sample;
}

e8::if_light::emission_surface_sample
e8::area_light::sample_emssion_surface(e8util::rng *rng) const {
    emission_surface_sample sample;
    sample.surface = m_geo->sample(rng);
    return sample;
}

e8util::vec3 e8::area_light::eval(e8util::vec3 const &i, e8util::vec3 const &n_light,
                                  e8util::vec3 const &n_target) const {
    float r2 = i.inner(i);
    e8util::vec3 i_norm = i / std::sqrt(r2);
    float cos_o = n_light.inner(i_norm);
    float cos_i = n_target.inner(-i_norm);
    if (cos_o > 0 && cos_i > 0) {
        return m_rad * cos_i * cos_o / r2;
    } else {
        return 0.0f;
    }
}

e8util::vec3 e8::area_light::projected_radiance(e8util::vec3 const &w,
                                                e8util::vec3 const &n) const {
    float cos = n.inner(w);
    if (cos > 0) {
        return m_rad * cos;
    } else {
        return 0.0f;
    }
}

e8util::vec3 e8::area_light::radiance(e8util::vec3 const &w, e8util::vec3 const &n) const {
    float cos = n.inner(w);
    if (cos > 0) {
        return m_rad;
    } else {
        return 0.0f;
    }
}

e8util::vec3 e8::area_light::power() const { return m_power; }

std::vector<e8::if_geometry const *> e8::area_light::geometries() const {
    return std::vector<e8::if_geometry const *>{m_geo.get()};
}

std::unique_ptr<e8::if_light> e8::area_light::copy() const {
    return std::make_unique<area_light>(*this);
}

std::unique_ptr<e8::if_light> e8::area_light::transform(e8util::mat44 const & /* trans */) const {
    return copy();
}

e8::sky_light::sky_light(std::string const &name, e8util::vec3 const &rad)
    : if_light(name), m_rad(rad) {}

e8::sky_light::sky_light(e8util::vec3 const &rad) : sky_light("Unkown_Sky_Light_Name", rad) {}

e8::sky_light::sky_light(sky_light const &other)
    : if_light(other.id(), other.name()), m_rad(other.m_rad), m_ref_p(other.m_ref_p),
      m_dia(other.m_dia) {}

void e8::sky_light::set_scene_boundary(e8util::aabb const &bbox) {
    m_dia = 2 * bbox.enclosing_radius();
    m_ref_p = bbox.centroid();
    m_ref_p(2) = 0.0f;
}

e8::if_light::emission_sample e8::sky_light::sample_emssion(e8util::rng *rng) const {
    emission_sample sample;

    e8util::vec3 z{0, 0, 1};
    e8util::vec3 u = e8util::vec3_cos_hemisphere_sample(z, rng->draw(), rng->draw());
    sample.w = -u;
    sample.solid_angle_dens = 1.0f;

    sample.surface.n = -u;
    sample.surface.p = (u + m_ref_p) * m_dia;
    sample.surface.area_dens = z.inner(u) / static_cast<float>(M_PI);

    return sample;
}

e8::if_light::emission_surface_sample
e8::sky_light::sample_emssion_surface(e8util::rng *rng) const {
    emission_surface_sample sample;

    e8util::vec3 z{0, 0, 1};
    e8util::vec3 u = e8util::vec3_cos_hemisphere_sample(z, rng->draw(), rng->draw());

    sample.surface.n = -u;
    sample.surface.p = (u + m_ref_p) * m_dia;
    sample.surface.area_dens = z.inner(u) / static_cast<float>(M_PI);

    return sample;
}

e8util::vec3 e8::sky_light::eval(e8util::vec3 const &i, e8util::vec3 const &n_light,
                                 e8util::vec3 const &n_target) const {
    e8util::vec3 i_norm = i.normalize();
    float cos_o = n_light.inner(i_norm);
    float cos_i = n_target.inner(-i_norm);
    return m_rad * std::max(cos_o, 0.0f) * std::max(cos_i, 0.0f);
}

e8util::vec3 e8::sky_light::projected_radiance(e8util::vec3 const &w, e8util::vec3 const &n) const {
    return m_rad * std::max(w.inner(n), 0.0f);
}

e8util::vec3 e8::sky_light::radiance(e8util::vec3 const &w, e8util::vec3 const &n) const {
    if (n.inner(w) > 0.0f) {
        return m_rad;
    } else {
        return 0.0f;
    }
}

e8util::vec3 e8::sky_light::power() const { return static_cast<float>(M_PI) * (m_dia * m_dia / 2); }

std::vector<e8::if_geometry const *> e8::sky_light::geometries() const {
    return std::vector<e8::if_geometry const *>();
}

std::unique_ptr<e8::if_light> e8::sky_light::copy() const {
    return std::make_unique<sky_light>(*this);
}

std::unique_ptr<e8::if_light> e8::sky_light::transform(e8util::mat44 const & /* trans */) const {
    return copy();
}
