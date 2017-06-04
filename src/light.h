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
        ~if_light();

        void                    bind_geometry(if_geometry const* geo);
        virtual e8util::vec3    sample() const = 0;

protected:
        if_geometry const*      m_geometry;
};

class area_light: if_light
{
public:
};

}

#endif // LIGHT_H
