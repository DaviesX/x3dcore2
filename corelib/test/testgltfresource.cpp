#include "testgltfresource.h"
#include "src/objdb.h"
#include "src/pathspace.h"
#include "src/resource.h"
#include <set>

test::test_gltf_resource::test_gltf_resource() {}

test::test_gltf_resource::~test_gltf_resource() {}

void test::test_gltf_resource::run() const
{
    e8util::gltf_scene res("res/polly/project_polly.gltf");

    e8::objdb db;
    db.register_manager(std::make_unique<e8::linear_path_space_layout>());
    db.push_updates();
    db.register_manager(std::make_unique<e8::bvh_path_space_layout>());
    db.push_updates();

    //        std::vector<e8::if_geometry*> const& geos = res.load_geometries();
    //        std::vector<e8::if_material*> const& mats = res.load_materials();
    //        std::vector<e8::if_light*> const& lights = res.load_lights();
    //        e8::if_camera* cam = res.load_camera();

    //        std::set<e8::if_material*> unique_mats;
    //        for (e8::if_material* mat: mats)
    //                unique_mats.insert(mat);
    //        std::set<e8::if_light*> unique_lights;
    //        for (e8::if_light* light: lights)
    //                unique_lights.insert(light);

    //        for (e8::if_geometry* geo: geos)
    //                delete geo;
    //        for (e8::if_material* mat: unique_mats)
    //                delete mat;
    //        for (e8::if_light* light: unique_lights)
    //                delete light;
    //        delete cam;
}
