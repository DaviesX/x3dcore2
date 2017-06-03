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


// trimesh
e8::trimesh::trimesh():
        m_aabb(0, 0)
{
}

e8::trimesh::~trimesh()
{
}

std::vector<e8util::vec3> const&
e8::trimesh::vertices() const
{
        return m_verts;
}

std::vector<e8util::vec3> const&
e8::trimesh::normals() const
{
        return m_norms;
}

std::vector<e8::triangle> const&
e8::trimesh::triangles() const
{
        return m_tris;
}

e8util::aabb
e8::trimesh::aabb() const
{
        return m_aabb;
}

void
e8::trimesh::compute_aabb()
{
        if (m_verts.empty())
                return;

        m_aabb = e8util::aabb(m_verts[0], m_verts[0]);
        for (unsigned i = 1; i < m_verts.size(); i ++) {
                m_aabb = m_aabb + m_verts[i];
        }
}


// sphere
e8::uv_sphere::uv_sphere(e8util::vec3 const& o, float r, unsigned const res)
{
        // vertex generation.
        // south pole.
        m_verts.push_back(e8util::vec3({0, 0, -r}) + o);

        // rings.
        for (unsigned j = 1; j < res - 1; j ++) {
                float phi = static_cast<float>(j)/res * M_PI - M_PI/2;

                for (unsigned i = 0; i < res; i ++) {
                        float theta = static_cast<float>(i)/res * 2*M_PI;

                        float x = std::cos(phi)*std::cos(theta);
                        float y = std::cos(phi)*std::sin(theta);
                        float z = std::sin(phi);
                        m_verts.push_back(r*e8util::vec3({x, y, z}) + o);
                        m_norms.push_back(e8util::vec3({x, y, z}));
                }
        }

        // north pole.
        m_verts.push_back(e8util::vec3({0, 0, r}) + o);

        // face generation.
        // connect the south pole with the first bottom ring.
        for (unsigned i = 1; i < res + 1; i ++) {
                unsigned v0 = 0;
                unsigned v1 = i;
                unsigned v2 = i%res + 1;

                m_tris.push_back(triangle({v2, v1, v0}));
        }

        // rings.
        for (unsigned j = 1; j < res - 2; j ++) {
                for (unsigned i = 1; i < res + 1; i ++) {
                        unsigned v0 = i + (j - 1)*res;
                        unsigned v1 = i + j*res;
                        unsigned v2 = i%res + 1 + j*res;

                        m_tris.push_back(triangle({v2, v1, v0}));

                        v0 = i + (j - 1)*res;
                        v1 = i%res + 1 + j*res;
                        v2 = i%res + 1 + (j - 1)*res;

                        m_tris.push_back(triangle({v2, v1, v0}));
                }
        }

        // connect the north pole with the first bottom ring.
        for (unsigned i = 1; i < res + 1; i ++) {
                unsigned v0 = i + (res - 3)*res;
                unsigned v1 = 1 + (res - 2)*res;
                unsigned v2 = i%res + 1 + (res - 3)*res;

                m_tris.push_back(triangle({v2, v1, v0}));
        }
}

e8::uv_sphere::~uv_sphere()
{
}
