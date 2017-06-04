#include "material.h"

e8::if_material::if_material()
{
}

e8::if_material::~if_material()
{
}



e8::oren_nayar::oren_nayar(e8util::vec3 const& albedo, float roughness):
        m_albedo(albedo), m_roughness(roughness)
{
}

e8util::vec3
e8::oren_nayar::eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const
{
}

e8util::vec3
e8::oren_nayar::sample(e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const
{
}



e8::cook_torr::cook_torr(e8util::vec3 const& albedo, float roughness, float ior):
        m_albedo(albedo), m_roughness(roughness), m_ior(ior)
{
}

e8util::vec3
e8::cook_torr::eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const
{
}

e8util::vec3
e8::cook_torr::sample(e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const
{
}
