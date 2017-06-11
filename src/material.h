#ifndef MATERIAL_H
#define MATERIAL_H


#include "tensor.h"

namespace e8
{

class if_material
{
public:
        if_material();
        virtual ~if_material();

        virtual e8util::vec3    eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const = 0;
        virtual e8util::vec3    sample(e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const = 0;
};


class oren_nayar: public if_material
{
public:
        oren_nayar(e8util::vec3 const& albedo, float roughness);

        e8util::vec3    eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const override;
        e8util::vec3    sample(e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const override;
private:
        e8util::vec3    m_albedo;
        e8util::vec3    m_roughness;
};


class cook_torr: public if_material
{
public:
        cook_torr(e8util::vec3 const& albedo, float roughness, float ior);

        e8util::vec3    eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const override;
        e8util::vec3    sample(e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const override;
private:
        e8util::vec3    m_albedo;
        float           m_roughness;
        float           m_ior;
};

}

#endif // IF_MATERIAL_H
