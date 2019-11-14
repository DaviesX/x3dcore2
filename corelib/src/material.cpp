#include "material.h"
#include <complex>
#include <memory>

namespace {

float fresnel(std::complex<float> const &ior, e8util::vec3 const &i, e8util::vec3 const &h) {
    float cos_th_flip = 1.0f - i.inner(h);
    float cos_th_flip2 = cos_th_flip * cos_th_flip;
    float cos_th_flip4 = cos_th_flip2 * cos_th_flip2;
    float cos_th_flip5 = cos_th_flip4 * cos_th_flip;
    float a = (ior.real() - 1) * (ior.real() - 1) + 4 * ior.real() * cos_th_flip5 +
              ior.imag() * ior.imag();
    float b = (ior.real() + 1) * (ior.real() + 1) + ior.imag() * ior.imag();
    return a / b;
}

float ggx_distri(float alpha2, e8util::vec3 const &n, e8util::vec3 const &h) {
    float cos_th = n.inner(h);
    float cos_th2 = cos_th * cos_th;
    float tan_th2 = (1.0f - cos_th2) / cos_th2;
    float c = 1.0f + tan_th2 / alpha2;
    return 1.0f / (static_cast<float>(M_PI) * alpha2 * cos_th2 * cos_th2 * c * c);
}

float lambda(float alpha2, e8util::vec3 const &n, e8util::vec3 const &w) {
    float cos_th = n.inner(w);
    float cos_th2 = cos_th * cos_th;
    float tan_th2 = (1.0f - cos_th2) / cos_th2;
    return (-1.0f + std::sqrt(1.0f + alpha2 * tan_th2)) / 2.0f;
}

float ggx_shadow(float alpha2, e8util::vec3 const &i, e8util::vec3 const &o,
                 e8util::vec3 const &n) {
    return 1.0f / (1.0f + lambda(alpha2, n, i) + lambda(alpha2, n, o));
}

} // namespace

e8::if_material::if_material(std::string const &name) : m_name(name) {}

e8::if_material::if_material(obj_id_t id, std::string const &name)
    : if_copyable_obj<e8::if_material>(id), m_name(name) {}

e8::if_material::~if_material() {}

std::string e8::if_material::name() const { return m_name; }

e8::obj_protocol e8::if_material::protocol() const { return obj_protocol::obj_protocol_material; }

e8::mat_fail_safe::mat_fail_safe(std::string const &name)
    : if_material(name), m_albedo{0.8f, 0.8f, 0.8f} {}

e8::mat_fail_safe::mat_fail_safe(mat_fail_safe const &other)
    : if_material(other.id(), other.name()), m_albedo(other.m_albedo) {}

std::unique_ptr<e8::if_material> e8::mat_fail_safe::copy() const {
    return std::make_unique<mat_fail_safe>(*this);
}

e8util::vec3 eval(e8util::vec2 const &uv, e8util::vec3 const &n, e8util::vec3 const &o,
                  e8util::vec3 const &i);
e8util::vec3 sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                    e8util::vec3 const &n, e8util::vec3 const &o);

e8util::vec3 e8::mat_fail_safe::eval(e8util::vec2 const & /*uv*/, e8util::vec3 const & /*n*/,
                                     e8util::vec3 const & /*o*/, e8util::vec3 const & /*i*/) const {
    return m_albedo * (1.0f / static_cast<float>(M_PI));
}

e8util::vec3 e8::mat_fail_safe::sample(e8util::rng *rng, float *cond_density,
                                       e8util::vec2 const & /*uv*/, e8util::vec3 const &n,
                                       e8util::vec3 const & /*o*/) const {
    e8util::vec3 const &i = e8util::vec3_cos_hemisphere_sample(n, rng->draw(), rng->draw());
    *cond_density = i.inner(n) / static_cast<float>(M_PI);
    return i;
}

e8::mat_mixture::mat_mixture(std::string const &name, std::unique_ptr<if_material> mat_0,
                             std::unique_ptr<if_material> mat_1, float ratio)
    : if_material(name), m_mat_0(std::move(mat_0)), m_mat_1(std::move(mat_1)), m_ratio(ratio) {}

e8::mat_mixture::mat_mixture(mat_mixture const &other)
    : if_material(other.id(), other.name()), m_mat_0(other.m_mat_0->copy()),
      m_mat_1(other.m_mat_1->copy()), m_ratio(other.m_ratio) {}

std::unique_ptr<e8::if_material> e8::mat_mixture::copy() const {
    return std::make_unique<mat_mixture>(*this);
}

e8util::vec3 e8::mat_mixture::eval(e8util::vec2 const &uv, e8util::vec3 const &n,
                                   e8util::vec3 const &o, e8util::vec3 const &i) const {
    return m_ratio * m_mat_0->eval(uv, n, o, i) + (1 - m_ratio) * m_mat_1->eval(uv, n, o, i);
}

e8util::vec3 e8::mat_mixture::sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                                     e8util::vec3 const &n, e8util::vec3 const &o) const {
    e8util::vec3 i;
    if (rng->draw() < m_ratio) {
        i = m_mat_0->sample(rng, cond_density, uv, n, o);
        *cond_density *= m_ratio;
    } else {
        i = m_mat_1->sample(rng, cond_density, uv, n, o);
        *cond_density *= 1 - m_ratio;
    }
    return i;
}

e8::oren_nayar::oren_nayar(std::string const &name, e8util::vec3 const &albedo, float roughness,
                           std::shared_ptr<texture_map<e8util::vec3>> const &albedo_map,
                           std::shared_ptr<texture_map<float>> const &roughness_map)
    : if_material(name), m_albedo_map(albedo_map), m_roughness_map(roughness_map), m_albedo(albedo),
      m_sigma(roughness) {
    float sigma2 = m_sigma * m_sigma;
    m_a = 1 - 0.5f * sigma2 / (sigma2 + 0.33f);
    m_b = 0.45f * sigma2 / (sigma2 + 0.09f);
}

e8::oren_nayar::oren_nayar(oren_nayar const &other)
    : if_material(other.id(), other.name()), m_albedo(other.m_albedo), m_sigma(other.m_sigma),
      m_a(other.m_a), m_b(other.m_b) {}

std::unique_ptr<e8::if_material> e8::oren_nayar::copy() const {
    return std::make_unique<oren_nayar>(*this);
}

e8util::vec3 e8::oren_nayar::albedo(e8util::vec2 const &uv) const {
    if (m_albedo_map != nullptr) {
        return m_albedo_map->map(uv);
    } else {
        return m_albedo;
    }
}

e8util::vec3 e8::oren_nayar::eval(e8util::vec2 const &uv, e8util::vec3 const &n,
                                  e8util::vec3 const &o, e8util::vec3 const &i) const {
    float cos_thei = i.inner(n);
    float cos_theo = o.inner(n);
    if (cos_thei <= 0 || cos_theo <= 0)
        return 0.0f;
    float cos_alpha, cos_beta;

    if (cos_thei < cos_theo) {
        cos_alpha = cos_thei;
        cos_beta = cos_theo;
    } else {
        cos_alpha = cos_theo;
        cos_beta = cos_thei;
    }

    float sin_alpha = std::sqrt(1.0f - cos_alpha * cos_alpha);
    float sin_beta = std::sqrt(1.0f - cos_beta * cos_beta);

    float cos_theio = cos_alpha * cos_beta + sin_alpha * sin_beta;
    float tan_beta = sin_beta / cos_theo;

    return albedo(uv) * (1.0f / static_cast<float>(M_PI)) *
           (m_a + m_b * std::max(0.0f, cos_theio) * sin_alpha * tan_beta);
}

e8util::vec3 e8::oren_nayar::sample(e8util::rng *rng, float *cond_density,
                                    e8util::vec2 const & /*uv*/, e8util::vec3 const &n,
                                    e8util::vec3 const & /*o*/) const {
    e8util::vec3 const &i = e8util::vec3_cos_hemisphere_sample(n, rng->draw(), rng->draw());
    *cond_density = i.inner(n) / static_cast<float>(M_PI);
    return i;
}

e8::cook_torr::cook_torr(std::string const &name, e8util::vec3 const &albedo, float roughness,
                         std::complex<float> const &ior,
                         std::shared_ptr<texture_map<e8util::vec3>> const &albedo_map,
                         std::shared_ptr<texture_map<float>> const &roughness_map)
    : if_material(name), m_albedo_map(albedo_map), m_roughness_map(roughness_map), m_albedo(albedo),
      m_ior(ior), m_alpha2(2 * roughness * roughness) {}

e8::cook_torr::cook_torr(cook_torr const &other)
    : if_material(other.id(), other.name()), m_albedo(other.m_albedo), m_ior(other.m_ior),
      m_alpha2(other.m_alpha2) {}

std::unique_ptr<e8::if_material> e8::cook_torr::copy() const {
    return std::make_unique<cook_torr>(*this);
}

e8util::vec3 e8::cook_torr::albedo(e8util::vec2 const &uv) const {
    return m_albedo_map != nullptr ? m_albedo_map->map(uv) : m_albedo;
}

float e8::cook_torr::alpha2(e8util::vec2 const &uv) const {
    if (m_roughness_map != nullptr) {
        float roughness = m_roughness_map->map(uv);
        return 2 * roughness * roughness;
    } else {
        return m_alpha2;
    }
}

e8util::vec3 e8::cook_torr::eval(e8util::vec2 const &uv, e8util::vec3 const &n,
                                 e8util::vec3 const &o, e8util::vec3 const &i) const {
    float cos_o_the = std::max(0.0f, n.inner(o));
    float cos_i_the = std::max(0.0f, n.inner(i));
    if (e8util::equals(cos_i_the, 0.0f) || e8util::equals(cos_o_the, 0.0f)) {
        // We know that no radiance can be emitted.
        return 0.0f;
    }

    e8util::vec3 h = i + o;
    if (e8util::equals(h, e8util::vec3{0.0f})) {
        // Degenerated case.
        return 0.0f;
    }
    h = h.normalize();

    float f = fresnel(m_ior, i, h);
    float roughness = alpha2(uv);
    float d = ggx_distri(roughness, n, h);
    float g = ggx_shadow(roughness, i, o, n);

    float c = f * d * g / (4.0f * cos_i_the * cos_o_the);
    return c * albedo(uv);
}

e8util::vec3 e8::cook_torr::sample(e8util::rng *rng, float *cond_density, e8util::vec2 const &uv,
                                   e8util::vec3 const &n, e8util::vec3 const &o) const {
    float ms_slope = alpha2(uv);

    // sample over the ggx distribution.
    e8util::vec3 u, v;
    e8util::vec3_basis(n, u, v);

    float theta = 2.0f * static_cast<float>(M_PI) * rng->draw();
    float t = rng->draw();
    float phi = std::atan(std::sqrt(ms_slope * t / (1.0f - t)));
    float sin_phi = std::sin(phi);
    float cos_phi = std::cos(phi);
    float x = sin_phi * std::cos(theta);
    float y = sin_phi * std::sin(theta);
    float z = cos_phi;

    e8util::vec3 h = x * u + y * v + z * n;
    float h_dot_o = h.inner(o);
    e8util::vec3 i_sample = 2.0f * h_dot_o * h - o;
    if (i_sample.inner(n) < 0.0f) {
        // Invalid sample. The ray shoots into the surface.
        *cond_density = 0.0f;
        return 0.0f;
    }
    *cond_density = ggx_distri(ms_slope, n, h) * cos_phi / (4.0f * h_dot_o);

    return i_sample;
}
