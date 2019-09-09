#include "lightsources.h"

e8::if_light_sources::if_light_sources() {}

e8::if_light_sources::~if_light_sources() {}

void e8::if_light_sources::load(if_obj const &obj, e8util::mat44 const &trans) {
    if_light const &light = static_cast<if_light const &>(obj);
    m_lights.insert(std::make_pair(obj.id(), light.transform(trans)));
}

void e8::if_light_sources::unload(if_obj const &obj) {
    auto it = m_lights.find(obj.id());
    if (it != m_lights.end()) {
        m_lights.erase(it);
    }
}

e8::obj_protocol e8::if_light_sources::support() const { return obj_protocol::obj_protocol_light; }

e8::basic_light_sources::basic_light_sources() {}

e8::basic_light_sources::~basic_light_sources() {}

e8::basic_light_sources::light_cdf::light_cdf(if_light const *light, float cum_power)
    : light(light), cum_power(cum_power) {}

void e8::basic_light_sources::commit() {
    m_light_cdf.clear();
    m_total_power = 0;
    for (std::pair<obj_id_t const, std::unique_ptr<if_light>> const &light : m_lights) {
        m_total_power += light.second->power().norm();
        m_light_cdf.push_back(light_cdf(light.second.get(), m_total_power));
    }
}

e8::if_light const *e8::basic_light_sources::sample_light(e8util::rng &rng, float &pdf) const {
    assert(!m_light_cdf.empty());
    float e = rng.draw() * m_total_power;
    unsigned lo = 0;
    unsigned hi = static_cast<unsigned>(m_light_cdf.size());
    while (lo < hi) {
        unsigned mi = (lo + hi) >> 1;
        if (m_light_cdf[mi].cum_power < e)
            lo = mi + 1;
        else
            hi = mi;
    }
    pdf = m_light_cdf[lo].light->power().norm() / m_total_power;
    return m_light_cdf[lo].light;
}
