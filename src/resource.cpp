#include <string>
#include <fstream>
#include "geometry.h"
#include "resource.h"


e8util::if_resource::if_resource()
{
}

e8util::if_resource::~if_resource()
{
}

bool
e8util::if_resource::save_geometries(std::vector<e8::if_geometry*> const&)
{
        return false;
}



std::vector<e8::if_geometry*>
e8util::cornell_scene::load_geometries() const
{
}

std::vector<e8::if_material*>
e8util::cornell_scene::load_materials() const
{
}

std::vector<e8::if_light*>
e8util::cornell_scene::load_lights() const
{
}



e8util::wavefront_obj::wavefront_obj(std::string const& location):
        m_location(location)
{
}

e8util::wavefront_obj::~wavefront_obj()
{
}

std::vector<e8::if_geometry*>
e8util::wavefront_obj::load_geometries() const
{
}

std::vector<e8::if_material*>
e8util::wavefront_obj::load_materials() const
{
}

std::vector<e8::if_light*>
e8util::wavefront_obj::load_lights() const
{
}

bool
e8util::wavefront_obj::save_geometries(std::vector<e8::if_geometry*> const& geometries)
{
        if (geometries.size() != 1)
                throw std::string("Can save only 1 geometry at a time.");

        std::ofstream file(m_location);

        if (!file.is_open()) {
                std::perror(("wavefront_obj::save_geometries to " + m_location).c_str());
                return false;
        }

        file << "# e8yescg wavefront_obj" << std::endl;

        e8::if_geometry* geo = geometries[0];

        // output vertices.
        std::vector<e8util::vec3> const& verts = geo->vertices();
        for (unsigned i = 0; i < verts.size(); i ++) {
                e8util::vec3 const& v = verts[i];
                file << "v " << v(0) << ' ' << v(1) << ' ' << v(2) << std::endl;
        }

        // output normals.
        std::vector<e8util::vec3> const& norms = geo->normals();
        for (unsigned i = 0; i < norms.size(); i ++) {
                e8util::vec3 const& n = norms[i];
                file << "vn " << n(0) << ' ' << n(1) << ' ' << n(2) << std::endl;
        }

        // output faces.
        std::vector<e8::triangle> const& faces = geo->triangles();
        for (unsigned i = 0; i < faces.size(); i ++) {
                e8::triangle const& f = faces[i] + 1;
                file << "f " << f(0) << ' ' << f(1) << ' ' << f(2) << std::endl;
        }

        return true;
}
