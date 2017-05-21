#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "tensor.h"

namespace e8
{

class if_geometry
{
public:
        if_geometry();
        ~if_geometry();
        virtual bool intersect(e8util::ray const& r, float& t) const = 0;
};

}

#endif // GEOMETRY_H
