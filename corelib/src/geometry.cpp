#include "tensor.h"
#include "geometry.h"



e8::if_geometry::if_geometry(std::string const& name):
        m_name(name)
{
}

e8::if_geometry::if_geometry(obj_id_t id, std::string const& name):
        if_operable_obj<if_geometry>(id),
        m_name(name)
{
}

e8::if_geometry::~if_geometry()
{
}

std::string
e8::if_geometry::name() const
{
        return m_name;
}

const std::type_info&
e8::if_geometry::interface() const
{
        return typeid(*this);
}

// trimesh
e8::trimesh::trimesh(std::string const& name):
        if_geometry(name),
        m_aabb(0.0f, 0.0f)
{
}

e8::trimesh::trimesh():
        trimesh("Unknown_Trimesh_Geometry_Name")
{
}

e8::trimesh::trimesh(trimesh const& mesh):
        if_geometry(id(), name())
{
        m_verts = mesh.m_verts;
        m_norms = mesh.m_norms;
        m_texcoords = mesh.m_texcoords;
        m_tris = mesh.m_tris;
        m_aabb = mesh.m_aabb;
        m_cum_area = mesh.m_cum_area;
        m_area = mesh.m_area;
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

void
e8::trimesh::sample(e8util::rng& rng, e8util::vec3& p, e8util::vec3& n, float& pdf) const
{
        // select a triangle.
        // @todo: use cdf.
        float q = rng.draw();
        unsigned i = static_cast<unsigned>(q*m_tris.size());

        e8::triangle const& t = m_tris[i];

        float u = rng.draw();
        float v = rng.draw();

        float r = std::sqrt(u);
        float b0 = 1 - r;
        float b1 = r*v;
        float b2 = 1 - b0 - b1;

        p = b0*m_verts[t(0)] + b1*m_verts[t(1)] + b2*m_verts[t(2)];
        n = (b0*m_norms[t(0)] + b1*m_norms[t(1)] + b2*m_norms[t(2)]).normalize();

        pdf = 1.0f/m_area;
}

float
e8::trimesh::surface_area() const
{
        return m_area;
}

e8util::aabb
e8::trimesh::aabb() const
{
        return m_aabb;
}

e8::trimesh*
e8::trimesh::transform(e8util::mat44 const& trans) const
{
        trimesh* transformed = new trimesh(*this);
        for (unsigned i = 0; i < transformed->m_verts.size(); i ++) {
                transformed->m_verts[i] = (trans*transformed->m_verts[i].homo(1.0f)).cart();
        }
        transformed->update_aabb();
        return transformed;
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
e8::trimesh::update_aabb()
{
        // compute aabb box.
        m_aabb = e8util::aabb(m_verts[0], m_verts[0]);
        for (unsigned i = 1; i < m_verts.size(); i ++) {
                m_aabb = m_aabb + m_verts[i];
        }
}

void
e8::trimesh::update_face_cdf()
{
        // compute area distribution.
        m_cum_area.resize(m_tris.size());
        float cum = 0;
        for (unsigned i = 0; i < m_tris.size(); i ++) {
                unsigned v0 = m_tris[i](0);
                unsigned v1 = m_tris[i](1);
                unsigned v2 = m_tris[i](2);
                float a = 0.5f*(m_verts[v1] - m_verts[v0]).outer(m_verts[v2] - m_verts[v0]).norm();
                cum += a;
                m_cum_area[i] = cum;
        }
        m_area = cum;
}

void
e8::trimesh::update()
{
        if (m_verts.empty())
                return;
        update_aabb();
        update_face_cdf();
}


// sphere
e8::uv_sphere::uv_sphere(std::string const& name,
                         e8util::vec3 const& o,
                         float r,
                         unsigned const res):
        trimesh(name)
{
        // vertex generation.
        // south pole.
        m_verts.push_back(e8util::vec3({0, 0, -r}) + o);
        m_norms.push_back(e8util::vec3({0, 0, -1}));
        m_texcoords.push_back((e8util::vec2({0, 0})));

        // rings.
        for (unsigned j = 1; j < res - 1; j ++) {
                float u = static_cast<float>(j)/res;
                float phi = u * static_cast<float>(M_PI) - static_cast<float>(M_PI/2);

                for (unsigned i = 0; i < res; i ++) {
                        float v = static_cast<float>(i)/res;
                        float theta = v * static_cast<float>(2*M_PI);

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

e8::uv_sphere::uv_sphere(e8util::vec3 const& o,
                         float r,
                         unsigned const res):
        uv_sphere("Unknown_Uv_Sphere_Trimesh_Geometry_Name",
                  o, r, res)
{
}

e8::uv_sphere::~uv_sphere()
{
}
