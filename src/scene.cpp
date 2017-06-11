#include "scene.h"

e8::if_scene::if_scene()
{
}

e8::if_scene::~if_scene()
{
        for (std::pair<if_geometry const*, binded_geometry> const& p: m_geometries)
                delete p.first;
        for (if_material const* mat: m_mats)
                delete mat;
        for (if_light const* light: m_lights)
                delete light;
}

void
e8::if_scene::add_geometry(if_geometry const* geometry)
{
        m_geometries.insert(
                std::pair<if_geometry const*, if_scene::binded_geometry>(geometry,
                                                                         if_scene::binded_geometry(geometry, nullptr, nullptr)));
}

void
e8::if_scene::add_material(if_material const* mat)
{
        m_mats.insert(mat);
}

void
e8::if_scene::add_light(if_light const* light)
{
        m_lights.insert(light);
}

void
e8::if_scene::bind(if_geometry const* geometry, if_material const* mat)
{
        m_geometries.at(geometry).mat = mat;
}

void
e8::if_scene::bind(if_geometry const* geometry, if_light const* light)
{
        m_geometries.at(geometry).light = light;
}

void
e8::if_scene::load(e8util::if_resource* res)
{
        std::vector<if_geometry*> const& geos = res->load_geometries();
        std::vector<if_material*> const& mats = res->load_materials();
        std::vector<if_light*> const& lights = res->load_lights();

        for (if_geometry* geo: geos)
                add_geometry(geo);
        for (if_material* mat: mats)
                add_material(mat);
        for (if_light* light: lights)
                add_light(light);
}



e8::linear_scene_layout::linear_scene_layout()
{
}

e8::linear_scene_layout::~linear_scene_layout()
{
}

void
e8::linear_scene_layout::update()
{
        m_cum_power.resize(m_lights.size());
        m_light_list.resize(m_lights.size());

        unsigned i = 0;
        m_total_power = 0;
        for (if_light const* light: m_lights) {
                m_total_power += light->power().norm();
                m_cum_power[i] = m_total_power;
                m_light_list[i] = light;
                i ++;
        }
}

e8::intersect_info
e8::linear_scene_layout::intersect(e8util::ray const& r) const
{
        float const t_min = 1e-4f;
        float const t_max = 1000.0f;

        float t = INFINITY;
        binded_geometry const*  hit_binded = nullptr;
        if_geometry const*      hit_geo = nullptr;
        triangle const*         hit_tri = nullptr;
        e8util::vec3            hit_b;

        for (std::pair<if_geometry const*, binded_geometry> const& p: m_geometries) {
                if_geometry const* geo = p.first;

                std::vector<e8util::vec3> const&        verts = geo->vertices();
                std::vector<triangle> const&            tris = geo->triangles();

                for (triangle const& tri: tris) {
                        e8util::vec3 const& v0 = verts[tri(0)];
                        e8util::vec3 const& v1 = verts[tri(1)];
                        e8util::vec3 const& v2 = verts[tri(2)];

                        float t0;
                        e8util::vec3 b;
                        if (r.intersect(v0, v1, v2, t_min, t_max, b, t0) && t0 < t) {
                                hit_b = b;
                                hit_geo = geo;
                                hit_binded = &p.second;
                                hit_tri = &tri;
                        }
                }
        }

        if (hit_geo != nullptr) {
                std::vector<e8util::vec3> const& verts = hit_geo->vertices();
                e8util::vec3 const& v0 = verts[(*hit_tri)(0)];
                e8util::vec3 const& v1 = verts[(*hit_tri)(1)];
                e8util::vec3 const& v2 = verts[(*hit_tri)(2)];
                e8util::vec3 const& vertex = hit_b(0)*v0 + hit_b(1)*v1 + hit_b(2)*v2;

                std::vector<e8util::vec3> const& normals = hit_geo->normals();
                e8util::vec3 const& n0 = normals[(*hit_tri)(0)];
                e8util::vec3 const& n1 = normals[(*hit_tri)(1)];
                e8util::vec3 const& n2 = normals[(*hit_tri)(2)];
                e8util::vec3 const& normal = (hit_b(0)*n0 + hit_b(1)*n1 + hit_b(2)*n2).normalize();
                return intersect_info(t, vertex, normal, hit_binded->mat, hit_binded->light);
        } else {
                return intersect_info();
        }
}

bool
e8::linear_scene_layout::has_intersect(e8util::ray const& r, float t_min, float t_max, float& t) const
{
        for (std::pair<if_geometry const*, binded_geometry> const& p: m_geometries) {
                if_geometry const* geo = p.first;

                std::vector<e8util::vec3> const&        verts = geo->vertices();
                std::vector<triangle> const&            tris = geo->triangles();

                for (triangle const& tri: tris) {
                        e8util::vec3 const& v0 = verts[tri(0)];
                        e8util::vec3 const& v1 = verts[tri(1)];
                        e8util::vec3 const& v2 = verts[tri(2)];

                        e8util::vec3 b;
                        if (r.intersect(v0, v1, v2, t_min, t_max, b, t)) {
                                return true;
                        }
                }
        }
        return false;
}

e8::batched_geometry
e8::linear_scene_layout::get_relevant_geometries(e8util::frustum const&) const
{
        throw std::string("Not implemented yet.");
}

std::vector<e8::if_light const*>
e8::linear_scene_layout::get_relevant_lights(e8util::frustum const&) const
{
        throw std::string("Not implemented yet.");
}

e8::if_light const*
e8::linear_scene_layout::sample_light(e8util::rng& rng, float& pdf) const
{
        float e = rng.draw()*m_total_power;
        unsigned lo = 0;
        unsigned hi = m_cum_power.size();
        while (lo < hi) {
                unsigned mi = (lo + hi) >> 1;
                if (m_cum_power[mi] < e)
                        lo = mi + 1;
                else
                        hi = mi;
        }
        pdf = m_light_list[lo]->power().norm()/m_total_power;
        return m_light_list[lo];
}
