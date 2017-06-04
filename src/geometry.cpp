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
        m_aabb(0.0f, 0.0f)
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

std::vector<e8util::vec2> const&
e8::trimesh::texcoords() const
{
        return m_texcoords;
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
e8::trimesh::vertices(std::vector<e8util::vec3> const& v)
{
        m_verts = v;
}

void
e8::trimesh::normals(std::vector<e8util::vec3> const& n)
{
        m_norms = n;
}

void
e8::trimesh::texcoords(std::vector<e8util::vec2> const& t)
{
        m_texcoords = t;
}

void
e8::trimesh::triangles(std::vector<triangle> const& t)
{
        m_tris = t;
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
        m_norms.push_back(e8util::vec3({0, 0, -1}));
        m_texcoords.push_back((e8util::vec2({0, 0})));

        // rings.
        for (unsigned j = 1; j < res - 1; j ++) {
                float u = static_cast<float>(j)/res;
                float phi = u * M_PI - M_PI/2;

                for (unsigned i = 0; i < res; i ++) {
                        float v = static_cast<float>(i)/res;
                        float theta = v * 2*M_PI;

                        float x = std::cos(phi)*std::cos(theta);
                        float y = std::cos(phi)*std::sin(theta);
                        float z = std::sin(phi);
                        m_verts.push_back(r*e8util::vec3({x, y, z}) + o);
                        m_norms.push_back(e8util::vec3({x, y, z}));
                        m_texcoords.push_back(e8util::vec2({u, v}));
                }
        }

        // north pole.
        m_verts.push_back(e8util::vec3({0, 0, r}) + o);
        m_norms.push_back(e8util::vec3({0, 0, 1}));
        m_texcoords.push_back((e8util::vec2({1, 1})));

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
