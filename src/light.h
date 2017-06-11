#ifndef LIGHT_H
#define LIGHT_H

#include "tensor.h"
#include "geometry.h"

namespace e8
{

class if_light
{
public:
        if_light();
        virtual ~if_light();

        virtual void            sample(e8util::rng& rng, float& pdf, e8util::vec3& p, e8util::vec3& n, e8util::vec3& w) const = 0;
        virtual e8util::vec3    eval(e8util::vec3 const& i, e8util::vec3 const& n) const = 0;
        virtual e8util::vec3    power() const = 0;
};

class area_light: public if_light
{
public:
        area_light(if_geometry const* geo, e8util::vec3 const& rad);
        void            sample(e8util::rng& rng, float& pdf, e8util::vec3& p, e8util::vec3& n, e8util::vec3& w) const override;
        e8util::vec3    eval(e8util::vec3 const& i, e8util::vec3 const& n) const override;
        e8util::vec3    power() const override;
private:
        if_geometry const*      m_geo;
        e8util::vec3            m_rad;
        e8util::vec3            m_power;
};

}

#endif // LIGHT_H
