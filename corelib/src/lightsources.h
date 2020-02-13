#ifndef LIGHTSOURCES_H
#define LIGHTSOURCES_H

#include "obj.h"
#include "tensor.h"
#include <map>
#include <memory>
#include <unordered_map>
#include <vector>

namespace e8 {
class if_light;
class if_geometry;
} // namespace e8

namespace e8 {

class if_light_sources : public if_obj_actuator {
  public:
    if_light_sources();
    virtual ~if_light_sources() override;

    void load(if_obj const &obj, e8util::mat44 const &trans) override;
    void unload(if_obj const &obj) override;
    obj_protocol support() const override;
    if_light const *obj_light(if_obj const &obj) const;

    virtual void commit() override = 0;
    virtual if_light const *sample_light(e8util::rng *rng, float *pdf) const = 0;
    virtual std::vector<if_light const *>
    get_relevant_lights(e8util::frustum const &frustum) const = 0;

  protected:
    // Store all loaded lights.
    std::map<obj_id_t, std::unique_ptr<if_light>> m_lights;

    // Fast lookup for lights that are attached an object.
    std::unordered_map<obj_id_t, if_light *const> m_obj_lights_lookup;
};

class basic_light_sources : public if_light_sources {
  public:
    basic_light_sources();
    ~basic_light_sources() override;

    std::vector<if_light const *>
    get_relevant_lights(e8util::frustum const &frustum) const override;
    if_light const *sample_light(e8util::rng *rng, float *prob_mass) const override;
    void commit() override;

  private:
    struct light_cdf {
        light_cdf(if_light const *light, float cum_power);
        if_light const *light;
        float cum_power;
    };

    std::vector<light_cdf> m_light_cdf;
    float m_total_power;
};

} // namespace e8

#endif // LIGHTSOURCES_H
