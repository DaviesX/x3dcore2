#include "src/pathtracer.h"
#include "src/renderer.h"
#include "src/resource.h"
#include "src/camera.h"
#include "src/scene.h"
#include "src/frame.h"
#include "src/compositor.h"
#include "testdirectrenderer.h"


test::test_direct_renderer::test_direct_renderer()
{
}

test::test_direct_renderer::~test_direct_renderer()
{
}

void
test::test_direct_renderer::run() const
{
        unsigned const width = 800;
        unsigned const height = 600;

        e8::pt_image_renderer r(new e8::pathtracer_factory(e8::pathtracer_factory::pt_type::direct,
                                                           e8::pathtracer_factory::options()));

        e8util::if_resource* res = new e8util::cornell_scene();
        e8::if_camera* cam = res->load_camera();

        e8::bvh_path_space_layout path_space;
        //e8::linear_path_space_layout path_space;
        path_space.load(res);
        path_space.commit();

        e8::img_file_frame img("test_direct.png", width, height);
        e8::aces_compositor com(width, height);
        com.enable_auto_exposure(false);
        com.exposure(1.0f);
        r.render(&path_space, cam, &com);

        com.commit(&img);
        img.commit();

        delete res;
        delete cam;
}
