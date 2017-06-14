#include "src/pathtracer.h"
#include "src/renderer.h"
#include "src/resource.h"
#include "src/camera.h"
#include "src/scene.h"
#include "src/frame.h"
#include "src/compositor.h"
#include "testrenderer.h"


test::test_renderer::test_renderer()
{
}

test::test_renderer::~test_renderer()
{
}

void
test::test_renderer::run() const
{
        unsigned const width = 133;
        unsigned const height = 100;

        e8::ol_image_renderer r(new e8::normal_pathtracer());

        e8util::if_resource* res = new e8util::cornell_scene();
        e8::if_camera* cam = res->load_camera();

        e8::bvh_scene_layout scene;
        scene.load(res);
        scene.update();

        e8::img_file_frame img("test.png", width, height);
        e8::aces_compositor com(width, height);
        r.render(&scene, cam, &com);

        com.commit(&img);
        img.commit();

        delete res;
        delete cam;
}
