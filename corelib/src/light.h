#ifndef LIGHT_H
#define LIGHT_H

#include "geometry.h"
#include "obj.h"
#include "tensor.h"
#include <memory>
#include <stdint.h>
#include <string>

namespace e8 {

class if_light : public if_operable_obj<if_light> {
  public:
    if_light(std::string const &name);
    virtual ~if_light() override;

    virtual void set_scene_boundary(e8util::aabb const &bbox);

    struct emission_surface_sample {
        if_geometry::surface_sample surface;
    };
    struct emission_direction_sample {
        e8util::vec3 w; // Emission direction.
        float solid_angle_dens;
    };
    struct emission_sample : public emission_surface_sample, emission_direction_sample {};

    virtual emission_sample sample_emssion(e8util::rng *rng) const = 0;
    virtual emission_surface_sample sample_emssion_surface(e8util::rng *rng) const = 0;

    virtual e8util::vec3 eval(e8util::vec3 const &i, e8util::vec3 const &n_light,
                              e8util::vec3 const &n_target) const = 0;
    virtual e8util::vec3 projected_radiance(e8util::vec3 const &w, e8util::vec3 const &n) const = 0;
    virtual e8util::vec3 radiance(e8util::vec3 const &w, e8util::vec3 const &n) const = 0;
    virtual e8util::vec3 power() const = 0;
    virtual std::unique_ptr<if_light> copy() const override = 0;
    virtual std::unique_ptr<if_light> transform(e8util::mat44 const &trans) const override = 0;

    std::string name() const;
    obj_protocol protocol() const override;

  protected:
    if_light(obj_id_t id, std::string const &name);
    std::string m_name;
};

class area_light : public if_light {
  public:
    area_light(std::string const &name, std::shared_ptr<if_geometry> const &geo,
               e8util::vec3 const &rad);
    area_light(area_light const &other);

    emission_sample sample_emssion(e8util::rng *rng) const override;
    emission_surface_sample sample_emssion_surface(e8util::rng *rng) const override;
    e8util::vec3 eval(e8util::vec3 const &i, e8util::vec3 const &n_light,
                      e8util::vec3 const &n_target) const override;
    e8util::vec3 projected_radiance(e8util::vec3 const &w, e8util::vec3 const &n) const override;
    e8util::vec3 radiance(e8util::vec3 const &w, e8util::vec3 const &n) const override;
    e8util::vec3 power() const override;
    std::unique_ptr<if_light> copy() const override;
    std::unique_ptr<if_light> transform(e8util::mat44 const &trans) const override;

  private:
    std::shared_ptr<if_geometry> m_geo;
    e8util::vec3 m_rad;
    e8util::vec3 m_power;
};

class sky_light : public if_light {
  public:
    sky_light(std::string const &name, e8util::vec3 const &rad);
    sky_light(e8util::vec3 const &rad);
    sky_light(sky_light const &other);

    void set_scene_boundary(e8util::aabb const &bbox) override;
    emission_sample sample_emssion(e8util::rng *rng) const override;
    emission_surface_sample sample_emssion_surface(e8util::rng *rng) const override;
    e8util::vec3 eval(e8util::vec3 const &i, e8util::vec3 const &n_light,
                      e8util::vec3 const &n_target) const override;
    e8util::vec3 projected_radiance(e8util::vec3 const &w, e8util::vec3 const &n) const override;
    e8util::vec3 radiance(e8util::vec3 const &w, e8util::vec3 const &n) const override;
    e8util::vec3 power() const override;
    std::unique_ptr<if_light> copy() const override;
    std::unique_ptr<if_light> transform(e8util::mat44 const &trans) const override;

  private:
    e8util::vec3 m_rad;
    e8util::vec3 m_ref_p;
    float m_dia;
    uint32_t m_padding;
};

} // namespace e8

#endif // LIGHT_H
