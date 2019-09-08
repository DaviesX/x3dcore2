#ifndef LIGHT_H
#define LIGHT_H

#include "geometry.h"
#include "obj.h"
#include "tensor.h"

namespace e8 {

class if_light : public if_operable_obj<if_light>
{
public:
    if_light(std::string const &name);
    virtual ~if_light() override;

    virtual void set_scene_boundary(e8util::aabb const &bbox);
    virtual void sample(e8util::rng &rng,
                        float &p_pdf,
                        float &w_pdf,
                        e8util::vec3 &p,
                        e8util::vec3 &n,
                        e8util::vec3 &w) const = 0;
    virtual void sample(e8util::rng &rng, float &pdf, e8util::vec3 &p, e8util::vec3 &n) const = 0;
    virtual e8util::vec3 eval(e8util::vec3 const &i, e8util::vec3 const &n) const = 0;
    virtual e8util::vec3 emission(e8util::vec3 const &w, e8util::vec3 const &n) const = 0;
    virtual e8util::vec3 power() const = 0;
    virtual std::unique_ptr<if_light> copy() const override = 0;
    virtual std::unique_ptr<if_light> transform(e8util::mat44 const &trans) const override = 0;

    std::string name() const;
    obj_protocol protocol() const override;

protected:
    if_light(obj_id_t id, std::string const &name);
    std::string m_name;
};

class area_light : public if_light
{
public:
    area_light(std::string const &name, if_geometry const &geo, e8util::vec3 const &rad);
    area_light(if_geometry const &geo, e8util::vec3 const &rad);
    area_light(area_light const &other);

    void sample(e8util::rng &rng,
                float &p_pdf,
                float &w_pdf,
                e8util::vec3 &p,
                e8util::vec3 &n,
                e8util::vec3 &w) const override;
    void sample(e8util::rng &rng, float &pdf, e8util::vec3 &p, e8util::vec3 &n) const override;
    e8util::vec3 eval(e8util::vec3 const &i, e8util::vec3 const &n) const override;
    e8util::vec3 emission(e8util::vec3 const &w, e8util::vec3 const &n) const override;
    e8util::vec3 power() const override;
    std::unique_ptr<if_light> copy() const override;
    std::unique_ptr<if_light> transform(e8util::mat44 const &trans) const override;

private:
    if_geometry const &m_geo;
    e8util::vec3 m_rad;
    e8util::vec3 m_power;
};

class sky_light : public if_light
{
public:
    sky_light(std::string const &name, e8util::vec3 const &rad);
    sky_light(e8util::vec3 const &rad);
    sky_light(sky_light const &other);

    void set_scene_boundary(e8util::aabb const &bbox) override;
    void sample(e8util::rng &rng,
                float &p_pdf,
                float &w_pdf,
                e8util::vec3 &p,
                e8util::vec3 &n,
                e8util::vec3 &w) const override;
    void sample(e8util::rng &rng, float &pdf, e8util::vec3 &p, e8util::vec3 &n) const override;
    e8util::vec3 eval(e8util::vec3 const &i, e8util::vec3 const &n) const override;
    e8util::vec3 emission(e8util::vec3 const &w, e8util::vec3 const &n) const override;
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
