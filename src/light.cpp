#include <cmath>
#include "light.h"

e8::if_light::if_light()
{

}

e8::if_light::~if_light()
{
}

e8::area_light::area_light(if_geometry const* geo, e8util::vec3 const& rad):
        m_geo(geo), m_power(static_cast<float>(M_PI)*m_geo->surface_area()*rad)
{
}

void
e8::area_light::sample(e8util::rng& rng, float& pdf, e8util::vec3& p, e8util::vec3& n, e8util::vec3& w) const
{
        m_geo->sample(rng, p, n, pdf);
}

e8util::vec3
e8::area_light::eval(e8util::vec3 const& i, e8util::vec3 const& n) const
{
        float r2 = i.inner(i);
        float cos = (1.0f/std::sqrt(r2)*i).inner(n);
        if (cos > 0)
                return cos/r2*m_rad;
        else
                return 0.0f;
}

e8util::vec3
e8::area_light::power() const
{
        return m_power;
}
