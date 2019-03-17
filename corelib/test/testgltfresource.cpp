#include <set>
#include "src/scene.h"
#include "src/resource.h"
#include "testgltfresource.h"


test::test_gltf_resource::test_gltf_resource()
{
}

test::test_gltf_resource::~test_gltf_resource()
{
}

void
test::test_gltf_resource::run() const
{
        e8util::gltf_scene res("res/polly/project_polly.gltf");

        e8::if_scene* linear_scene = new e8::linear_scene_layout();
        linear_scene->load(&res);
        linear_scene->update();
        delete linear_scene;

        e8::if_scene* bvh_scene = new e8::bvh_scene_layout();
        bvh_scene->load(&res);
        bvh_scene->update();
        delete bvh_scene;

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
