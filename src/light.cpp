#include <cmath>
#include "tensor.h"
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
        e8util::vec3 u, v;
        e8util::vec3_basis(n, u, v);

        float e0 = rng.draw()*2*M_PI;
        float e1 = rng.draw();

        float z = std::sqrt(e1);
        float r = std::sqrt(1.0f - z*z);
        float x = r*std::cos(e0);
        float y = r*std::sin(e0);

        w = x*u + y*v + z*n;
        pdf = n.inner(w)/M_PI;
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
