#ifndef MATERIAL_H
#define MATERIAL_H


#include "tensor.h"

namespace e8
{

class if_material
{
public:
        if_material(std::string const& name);
        virtual ~if_material();

        std::string             name() const;

        virtual e8util::vec3    eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const = 0;
        virtual e8util::vec3    sample(e8util::rng& rng, e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const = 0;
private:
        std::string     m_name;
};


class oren_nayar: public if_material
{
public:
        oren_nayar(e8util::vec3 const& albedo, float roughness);
        oren_nayar(std::string const& name, e8util::vec3 const& albedo, float roughness);

        e8util::vec3    eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const override;
        e8util::vec3    sample(e8util::rng& rng, e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const override;
private:
        e8util::vec3    m_albedo;
        float           m_sigma;
        float           A;
        float           B;
};


class cook_torr: public if_material
{
public:
        cook_torr(std::string const& name,
                  e8util::vec3 const& albedo,
                  float beta,
                  float ior);
        cook_torr(e8util::vec3 const& albedo,
                  float beta,
                  float ior);

        e8util::vec3    eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const override;
        e8util::vec3    sample(e8util::rng& rng, e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const override;
private:
        float           fresnel(e8util::vec3 const& i, e8util::vec3 const& h) const;
        float           ggx_distri(e8util::vec3 const& n, e8util::vec3 const& h) const;
        float           ggx_shadow1(e8util::vec3 const& v, e8util::vec3 const& h) const;
        float           ggx_shadow(e8util::vec3 const& i, e8util::vec3 const& o, e8util::vec3 const& h) const;

        e8util::vec3    m_albedo;
        float           m_beta2;
        float           m_ior2;
};

}

#endif // IF_MATERIAL_H
