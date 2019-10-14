#include "material.h"
#include <memory>

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

e8util::vec3 e8::mat_fail_safe::eval(e8util::vec3 const & /* n */, e8util::vec3 const & /* o */,
                                     e8util::vec3 const & /* i */) const {
    return m_albedo * (1.0f / static_cast<float>(M_PI));
}

e8util::vec3 e8::mat_fail_safe::sample(e8util::rng &rng, e8util::vec3 const &n,
                                       e8util::vec3 const & /* o */, float &pdf) const {
    e8util::vec3 const &i = e8util::vec3_cos_hemisphere_sample(n, rng.draw(), rng.draw());
    pdf = i.inner(n) / static_cast<float>(M_PI);
    return i;
}

e8::oren_nayar::oren_nayar(std::string const &name, e8util::vec3 const &albedo, float roughness)
    : if_material(name), m_albedo(albedo), m_sigma(roughness) {
    float sigma2 = m_sigma * m_sigma;
    A = 1 - 0.5f * sigma2 / (sigma2 + 0.33f);
    B = 0.45f * sigma2 / (sigma2 + 0.09f);
}

e8::oren_nayar::oren_nayar(oren_nayar const &other)
    : if_material(other.id(), other.name()), m_albedo(other.m_albedo), m_sigma(other.m_sigma),
      A(other.A), B(other.B) {}

std::unique_ptr<e8::if_material> e8::oren_nayar::copy() const {
    return std::make_unique<oren_nayar>(*this);
}

e8util::vec3 e8::oren_nayar::eval(e8util::vec3 const &n, e8util::vec3 const &o,
                                  e8util::vec3 const &i) const {
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

    return m_albedo * (1.0f / static_cast<float>(M_PI)) *
           (A + B * std::max(0.0f, cos_theio) * sin_alpha * tan_beta);
}

e8util::vec3 e8::oren_nayar::sample(e8util::rng &rng, e8util::vec3 const &n,
                                    e8util::vec3 const & /* o */, float &pdf) const {
    e8util::vec3 const &i = e8util::vec3_cos_hemisphere_sample(n, rng.draw(), rng.draw());
    pdf = i.inner(n) / static_cast<float>(M_PI);
    return i;
}

e8::cook_torr::cook_torr(std::string const &name, e8util::vec3 const &albedo, float beta, float ior)
    : if_material(name), m_albedo(albedo), m_beta2(beta * beta), m_ior2(ior * ior) {}

e8::cook_torr::cook_torr(cook_torr const &other)
    : if_material(other.id(), other.name()), m_albedo(other.m_albedo), m_beta2(other.m_beta2),
      m_ior2(other.m_ior2) {}

std::unique_ptr<e8::if_material> e8::cook_torr::copy() const {
    return std::make_unique<cook_torr>(*this);
}

float e8::cook_torr::fresnel(e8util::vec3 const &i, e8util::vec3 const &h) const {
    float c = i.inner(h);
    float g = std::sqrt(m_ior2 - 1.0f + c * c);
    float f = (g - c) / (g + c);
    float d = (c * (g + c) - 1.0f) / (c * (g - c) + 1.0f);
    return 0.5f * f * f * (1.0f + d * d);
}

float e8::cook_torr::ggx_distri(e8util::vec3 const &n, e8util::vec3 const &h) const {
    float cos_th = n.inner(h);
    float cos_th2 = cos_th * cos_th;
    float tan_th2 = 1.0f / cos_th2 - 1.0f;
    float c = m_beta2 + tan_th2;
    return m_beta2 / (static_cast<float>(M_PI) * cos_th2 * cos_th2 * c * c);
}

float e8::cook_torr::ggx_shadow1(e8util::vec3 const &v, e8util::vec3 const &h) const {
    float cos_vh = v.inner(h);
    float tan_tv2 = 1.0f / (cos_vh * cos_vh) - 1.0f;
    return 2.0f / (1.0f + std::sqrt(1.0f + m_beta2 * tan_tv2));
}

float e8::cook_torr::ggx_shadow(e8util::vec3 const &i, e8util::vec3 const &o,
                                e8util::vec3 const &h) const {
    return ggx_shadow1(i, h) * ggx_shadow1(o, h);
}

e8util::vec3 e8::cook_torr::eval(e8util::vec3 const &n, e8util::vec3 const &o,
                                 e8util::vec3 const &i) const {
    float cos_the2 = n.inner(o);
    float cos_the = n.inner(i);
    if (cos_the2 <= 0 || cos_the <= 0)
        return 0.0f;
    e8util::vec3 const &h = (i + o).normalize();

    float F = fresnel(i, h);
    float D = ggx_distri(n, h);
    float G = ggx_shadow(i, o, n);

    float c = (1 - F) * D * G * (1.0f / (4.0f * cos_the * cos_the2));
    return c * m_albedo;
}

e8util::vec3 e8::cook_torr::sample(e8util::rng &rng, e8util::vec3 const &n, e8util::vec3 const &o,
                                   float &pdf) const {
    // sample over the ggx distribution.
    e8util::vec3 u, v;
    e8util::vec3_basis(n, u, v);

    float theta = 2.0f * static_cast<float>(M_PI) * rng.draw();
    float t = rng.draw();
    float phi = std::atan(std::sqrt(m_beta2 * t / (1.0f - t)));
    float sin_phi = std::sin(phi);
    float cos_phi = std::cos(phi);
    float x = sin_phi * std::cos(theta);
    float y = sin_phi * std::sin(theta);
    float z = cos_phi;

    e8util::vec3 const &m = u * x + v * y + n * z;
    float m_dot_o = m.inner(o);
    pdf = ggx_distri(n, m) * cos_phi / (4.0f * m_dot_o);

    return 2.0f * m_dot_o * m - o;
}
