#ifndef GEOMETRY_H
#define GEOMETRY_H

#include "tensor.h"

namespace e8
{

typedef e8util::vec<3, unsigned> triangle;

class if_geometry
{
public:
        if_geometry();
        ~if_geometry();

        virtual std::vector<e8util::vec3> const&        vertices() const = 0;
        virtual std::vector<e8util::vec3> const&        normals() const = 0;
        virtual std::vector<triangle> const&            triangles() const = 0;
        virtual e8util::aabb                            aabb() const = 0;
};

}

#endif // GEOMETRY_H
