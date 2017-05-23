#ifndef MATERIAL_H
#define MATERIAL_H


#include "tensor.h"

namespace e8
{

class if_material
{
public:
        if_material();
        ~if_material();

        virtual e8util::vec3    eval(e8util::vec3 const &n, e8util::vec3 const &o, e8util::vec3 const &i) const = 0;
        virtual e8util::vec3    sample(e8util::vec3 const &n, e8util::vec3 const &o, float& pdf) const = 0;
};

}

#endif // IF_MATERIAL_H
