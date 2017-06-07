#include "scene.h"

e8::if_scene::if_scene()
{
}

e8::if_scene::~if_scene()
{
}

std::vector<e8::if_light const*>
e8::if_scene::get_lights() const
{
        std::vector<e8::if_light const*> lights;
        for (e8::if_light const* light: m_lights)
                lights.push_back(light);
        return lights;
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
