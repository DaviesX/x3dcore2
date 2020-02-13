#include "geometry.h"
#include "tensor.h"
#include <cmath>
#include <ext/alloc_traits.h>

e8::if_geometry::if_geometry(std::string const &name) : if_operable_obj<if_geometry>(name) {}

e8::if_geometry::if_geometry(if_geometry const &other)
    : if_operable_obj<if_geometry>(other.id(), other.name()), m_mat_id(other.m_mat_id) {}

e8::if_geometry::~if_geometry() {}

e8::obj_protocol e8::if_geometry::protocol() const { return obj_protocol::obj_protocol_geometry; }

void e8::if_geometry::attach_material(obj_id_t mat_id) { m_mat_id = mat_id; }

std::optional<e8::obj_id_t> e8::if_geometry::material_id() const { return m_mat_id; }

// trimesh
e8::trimesh::trimesh(std::string const &name) : if_geometry(name), m_aabb(0.0f, 0.0f) {}

e8::trimesh::trimesh() : trimesh("Unknown_Trimesh_Geometry_Name") {}

e8::trimesh::trimesh(trimesh const &mesh) : if_geometry(mesh) {
    m_verts = mesh.m_verts;
    m_norms = mesh.m_norms;
    m_texcoords = mesh.m_texcoords;
    m_tris = mesh.m_tris;
    m_aabb = mesh.m_aabb;
    m_cum_area = mesh.m_cum_area;
    m_area = mesh.m_area;
}

e8::trimesh::~trimesh() {}

std::vector<e8util::vec3> const &e8::trimesh::vertices() const { return m_verts; }

std::vector<e8util::vec3> const &e8::trimesh::normals() const { return m_norms; }

std::vector<e8util::vec2> const &e8::trimesh::texcoords() const { return m_texcoords; }

std::vector<e8::triangle> const &e8::trimesh::triangles() const { return m_tris; }

e8::if_geometry::surface_sample e8::trimesh::sample(e8util::rng *rng) const {
    // select a triangle.
    // @todo: use cdf.
    float q = rng->draw();
    unsigned i = static_cast<unsigned>(q * m_tris.size());

    e8::triangle const &t = m_tris[i];

    float u = rng->draw();
    float v = rng->draw();

    float r = std::sqrt(u);
    float b0 = 1 - r;
    float b1 = r * v;
    float b2 = 1 - b0 - b1;

    surface_sample sample;
    sample.p = b0 * m_verts[t(0)] + b1 * m_verts[t(1)] + b2 * m_verts[t(2)];
    sample.n = (b0 * m_norms[t(0)] + b1 * m_norms[t(1)] + b2 * m_norms[t(2)]).normalize();
    sample.area_dens = 1.0f / m_area;

    return sample;
}

float e8::trimesh::surface_area() const { return m_area; }

e8util::aabb e8::trimesh::aabb() const { return m_aabb; }

std::unique_ptr<e8::if_geometry> e8::trimesh::copy() const {
    return std::make_unique<trimesh>(*this);
}

std::unique_ptr<e8::if_geometry> e8::trimesh::transform(e8util::mat44 const &trans) const {
    std::unique_ptr<trimesh> transformed = std::make_unique<trimesh>(*this);
    for (unsigned i = 0; i < transformed->m_verts.size(); i++) {
        transformed->m_verts[i] = (trans * transformed->m_verts[i].homo(1.0f)).cart();
    }
    e8util::mat44 normal_trans = ~(trans ^ (-1));
    for (unsigned i = 0; i < transformed->m_verts.size(); i++) {
        transformed->m_norms[i] = (normal_trans * transformed->m_norms[i].homo(1.0f)).cart();
    }
    transformed->update_aabb();
    return transformed;
}

void e8::trimesh::vertices(std::vector<e8util::vec3> const &v) { m_verts = v; }

void e8::trimesh::normals(std::vector<e8util::vec3> const &n) { m_norms = n; }

void e8::trimesh::texcoords(std::vector<e8util::vec2> const &t) { m_texcoords = t; }

void e8::trimesh::triangles(std::vector<triangle> const &t) { m_tris = t; }

void e8::trimesh::update_aabb() {
    // compute aabb box.
    m_aabb = e8util::aabb(m_verts[0], m_verts[0]);
    for (unsigned i = 1; i < m_verts.size(); i++) {
        m_aabb = m_aabb + m_verts[i];
    }
}

void e8::trimesh::update_face_cdf() {
    // compute area distribution.
    m_cum_area.resize(m_tris.size());
    float cum = 0;
    for (unsigned i = 0; i < m_tris.size(); i++) {
        unsigned v0 = m_tris[i](0);
        unsigned v1 = m_tris[i](1);
        unsigned v2 = m_tris[i](2);
        float a = 0.5f * (m_verts[v1] - m_verts[v0]).outer(m_verts[v2] - m_verts[v0]).norm();
        cum += a;
        m_cum_area[i] = cum;
    }
    m_area = cum;
}

void e8::trimesh::update() {
    if (m_verts.empty())
        return;
    update_aabb();
    update_face_cdf();
}

e8::triangle_fragment::triangle_fragment(std::string const &name, e8util::vec3 const &a,
                                         e8util::vec3 const &b, e8util::vec3 const &c)
    : trimesh(name) {
    vertices(std::vector<e8util::vec3>{a, b, c});

    e8util::vec3 normal = (c - a).outer(b - a).normalize();
    normals(std::vector<e8util::vec3>{normal, normal, normal});

    e8util::vec3 ori = (a + b + c) / 3.0f;
    e8util::vec3 t_a = a - ori;
    e8util::vec3 t_b = b - ori;
    e8util::vec3 t_c = c - ori;
    e8util::aabb bbox = e8util::aabb(t_a, t_a) + e8util::aabb(t_b, t_b) + e8util::aabb(t_c, t_c);
    t_a -= bbox.min();
    t_b -= bbox.min();
    t_c -= bbox.min();
    e8util::vec3 plane_vec = bbox.max() - bbox.min();
    e8util::vec3 x_basis;
    if (!e8util::equals(plane_vec(0), 0.0f)) {
        x_basis = e8util::vec3{1.0f, 0, 0};
    } else {
        x_basis = e8util::vec3{0, 1.0f, 0};
    }
    x_basis *= plane_vec.inner(x_basis);
    e8util::vec3 y_basis = plane_vec - x_basis;
    texcoords(std::vector<e8util::vec2>{{t_a.inner(x_basis), t_a.inner(y_basis)},
                                        {t_b.inner(x_basis), t_b.inner(y_basis)},
                                        {t_c.inner(x_basis), t_c.inner(y_basis)}});

    triangles(std::vector<triangle>{e8util::vec<3, unsigned>{0, 1, 2}});
}

e8::triangle_fragment::~triangle_fragment() {}

// sphere
e8::uv_sphere::uv_sphere(std::string const &name, e8util::vec3 const &o, float r,
                         unsigned const res, bool flip_normal)
    : trimesh(name) {
    // vertex generation.
    // south pole.
    m_verts.push_back(e8util::vec3({0, 0, -r}) + o);
    if (!flip_normal) {
        m_norms.push_back(e8util::vec3({0, 0, -1}));
    } else {
        m_norms.push_back(e8util::vec3({0, 0, 1}));
    }
    m_texcoords.push_back((e8util::vec2({0, 0})));

    // rings.
    for (unsigned j = 1; j < res - 1; j++) {
        float u = static_cast<float>(j) / res;
        float phi = (1 - u) * static_cast<float>(M_PI);

        for (unsigned i = 0; i < res; i++) {
            float v = static_cast<float>(i) / res;
            float theta = v * static_cast<float>(2 * M_PI);

            float x = std::sin(phi) * std::cos(theta);
            float y = std::sin(phi) * std::sin(theta);
            float z = std::cos(phi);
            m_verts.push_back(r * e8util::vec3({x, y, z}) + o);
            if (!flip_normal) {
                m_norms.push_back(e8util::vec3({x, y, z}));
            } else {
                m_norms.push_back(-1.0f * e8util::vec3({x, y, z}));
            }
            m_texcoords.push_back(e8util::vec2({u, v}));
        }
    }

    // north pole.
    m_verts.push_back(e8util::vec3({0, 0, r}) + o);
    if (!flip_normal) {
        m_norms.push_back(e8util::vec3({0, 0, 1}));
    } else {
        m_norms.push_back(e8util::vec3({0, 0, -1}));
    }
    m_texcoords.push_back((e8util::vec2({1, 1})));

    // face generation.
    // connect the south pole with the first bottom ring.
    for (unsigned i = 1; i < res + 1; i++) {
        unsigned v0 = 0;
        unsigned v1 = i;
        unsigned v2 = i % res + 1;

        m_tris.push_back(triangle({v2, v1, v0}));
    }

    // rings.
    for (unsigned j = 1; j < res - 2; j++) {
        for (unsigned i = 1; i < res + 1; i++) {
            unsigned v0 = i + (j - 1) * res;
            unsigned v1 = i + j * res;
            unsigned v2 = i % res + 1 + j * res;

            m_tris.push_back(triangle({v2, v1, v0}));

            v0 = i + (j - 1) * res;
            v1 = i % res + 1 + j * res;
            v2 = i % res + 1 + (j - 1) * res;

            m_tris.push_back(triangle({v2, v1, v0}));
        }
    }

    // connect the north pole with the first bottom ring.
    for (unsigned i = 1; i < res + 1; i++) {
        unsigned v0 = i + (res - 3) * res;
        unsigned v1 = 1 + (res - 2) * res;
        unsigned v2 = i % res + 1 + (res - 3) * res;

        m_tris.push_back(triangle({v2, v1, v0}));
    }
}

e8::uv_sphere::~uv_sphere() {}
