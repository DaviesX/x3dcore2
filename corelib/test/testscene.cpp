#include <iostream>
#include "src/pathtracer.h"
#include "src/pathtracerfact.h"
#include "src/renderer.h"
#include "src/resource.h"
#include "src/camera.h"
#include "src/scene.h"
#include "src/frame.h"
#include "src/compositor.h"
#include "testscene.h"


test::test_scene::test_scene()
{
}

test::test_scene::~test_scene()
{
}

void
test::test_scene::run() const
{
        unsigned const width = 1024;
        unsigned const height = 768;

        e8::pt_image_renderer r(new e8::pathtracer_factory(e8::pathtracer_factory::pt_type::normal,
                                                           e8::pathtracer_factory::options()));

        e8util::if_resource* res = new e8util::cornell_scene();
        e8::if_camera* cam = res->load_camera();

        //e8::linear_scene_layout scene;
        e8::bvh_scene_layout scene;
        scene.load(res);
        scene.commit();

        std::cout << "max_depth: " << scene.max_depth() << std::endl;
        std::cout << "avg_depth: " << scene.avg_depth() << std::endl;
        std::cout << "dev_depth: " << scene.dev_depth() << std::endl;
        std::cout << "num_nodes: " << scene.num_nodes() << std::endl;


        e8::img_file_frame img("test.png", width, height);
        e8::aces_compositor com(width, height);
        com.enable_auto_exposure(false);
        com.exposure(1.0f);
        r.render(&scene, cam, &com);

        com.commit(&img);
        img.commit();

        delete res;
        delete cam;
}
