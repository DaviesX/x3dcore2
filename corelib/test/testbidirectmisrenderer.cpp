#include "src/pathtracer.h"
#include "src/renderer.h"
#include "src/resource.h"
#include "src/camera.h"
#include "src/scene.h"
#include "src/frame.h"
#include "src/compositor.h"
#include "testbidirectmisrenderer.h"


test::test_bidirect_mis_renderer::test_bidirect_mis_renderer()
{
}


test::test_bidirect_mis_renderer::~test_bidirect_mis_renderer()
{
}

void
test::test_bidirect_mis_renderer::run() const
{
        unsigned const width = 800;
        unsigned const height = 600;

        e8::pt_image_renderer r(new e8::pathtracer_factory(e8::pathtracer_factory::pt_type::bidirect_mis,
                                                           e8::pathtracer_factory::options()));

        e8util::if_resource* res = new e8util::cornell_scene();
        e8::if_camera* cam = res->load_camera();

        e8::bvh_path_space_layout scene;
        //e8::linear_scene_layout scene;
        scene.load(res);
        scene.commit();

        e8::img_file_frame img("test_bidirect_mis.png", width, height);
        e8::clamp_compositor com(width, height);

        for (unsigned i = 0; i < 10; i ++)
                r.render(&scene, cam, &com);

        com.commit(&img);
        img.commit();

        delete res;
        delete cam;
}
