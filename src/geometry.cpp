#include "geometry.h"

e8::if_geometry::if_geometry()
{

}

e8::if_geometry::~if_geometry()
{
}

void
e8::if_geometry::bind(if_material const& mat)
{
        m_mat = &mat;
}

e8::if_material const&
e8::if_geometry::material() const
{
        return *m_mat;
}


e8::sphere::sphere(e8util::vec3 const& o, float r, unsigned res):
        m_o(o), m_r(r), m_res(res)
{
}

e8::sphere::~sphere()
{
}

std::vector<e8util::vec3> const&
e8::sphere::vertices() const
{
}

std::vector<e8util::vec3> const&
e8::sphere::normals() const
{
}

std::vector<e8::triangle> const&
e8::sphere::triangles() const
{
}

e8util::aabb
e8::sphere::aabb() const
{
}
