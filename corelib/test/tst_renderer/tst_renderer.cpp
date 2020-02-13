#include "src/camera.h"
#include "src/cameracontainer.h"
#include "src/compositor.h"
#include "src/frame.h"
#include "src/lightsources.h"
#include "src/pathspace.h"
#include "src/renderer.h"
#include "src/resource.h"
#include <QString>
#include <QtTest>
#include <cmath>
#include <memory>
#include <string>

class tst_renderer : public QObject {
    Q_OBJECT

  public:
    tst_renderer() = default;
    ~tst_renderer() = default;

  private slots:
    void pt_render_cornel_balls();
};

struct cornell_balls {
    cornell_balls() = default;
    ~cornell_balls() = default;
    cornell_balls(cornell_balls &&) = default;

    std::unique_ptr<e8::if_path_space> path_space;
    std::unique_ptr<e8::if_light_sources> light_sources;
    std::unique_ptr<e8::if_material_container> mats;
    std::unique_ptr<e8::if_camera> camera;
};

cornell_balls cornell_box_path_space() {
    cornell_balls scene;
    scene.path_space = std::make_unique<e8::bvh_path_space_layout>();
    scene.light_sources = std::make_unique<e8::basic_light_sources>();
    scene.mats = std::make_unique<e8::default_material_container>();

    std::shared_ptr<e8::if_material> white =
        std::make_shared<e8::oren_nayar>("white", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    std::shared_ptr<e8::if_material> red =
        std::make_shared<e8::oren_nayar>("red", e8util::vec3({0.630f, 0.065f, 0.050f}), 0.078f);
    std::shared_ptr<e8::if_material> green =
        std::make_shared<e8::oren_nayar>("green", e8util::vec3({0.140f, 0.450f, 0.091f}), 0.078f);
    std::shared_ptr<e8::if_material> glossy = std::make_shared<e8::cook_torr>(
        "glossy", e8util::vec3(0.95f), 0.2f, std::complex<float>(2.93f, 3.0f));
    std::shared_ptr<e8::if_material> light_mat =
        std::make_shared<e8::oren_nayar>("light", e8util::vec3({0, 0, 0}), 0.078f);

    scene.mats->load(*white, e8util::mat44_scale(1.0f));
    scene.mats->load(*red, e8util::mat44_scale(1.0f));
    scene.mats->load(*green, e8util::mat44_scale(1.0f));
    scene.mats->load(*glossy, e8util::mat44_scale(1.0f));
    scene.mats->load(*light_mat, e8util::mat44_scale(1.0f));
    scene.mats->commit();

    std::shared_ptr<e8::if_geometry> left_wall =
        e8util::wavefront_obj("testdata/cornellbox/left_wall.obj").load_geometry();
    left_wall->attach_material(red->id());

    std::shared_ptr<e8::if_geometry> right_wall =
        e8util::wavefront_obj("testdata/cornellbox/right_wall.obj").load_geometry();
    right_wall->attach_material(green->id());

    std::shared_ptr<e8::if_geometry> back_wall =
        e8util::wavefront_obj("testdata/cornellbox/back_wall.obj").load_geometry();
    back_wall->attach_material(white->id());

    std::shared_ptr<e8::if_geometry> ceiling =
        e8util::wavefront_obj("testdata/cornellbox/ceiling.obj").load_geometry();
    ceiling->attach_material(white->id());

    std::shared_ptr<e8::if_geometry> floor =
        e8util::wavefront_obj("testdata/cornellbox/floor.obj").load_geometry();
    floor->attach_material(white->id());

    std::shared_ptr<e8::if_geometry> left_sphere =
        e8util::wavefront_obj("testdata/cornellbox/left_sphere.obj").load_geometry();
    left_sphere->attach_material(glossy->id());

    std::shared_ptr<e8::if_geometry> right_sphere =
        e8util::wavefront_obj("testdata/cornellbox/right_sphere.obj").load_geometry();
    right_sphere->attach_material(white->id());

    std::shared_ptr<e8::if_geometry> light_geo =
        e8util::wavefront_obj("testdata/cornellbox/light.obj").load_geometry();
    light_geo->attach_material(light_mat->id());

    scene.path_space->load(*left_wall, e8util::mat44_scale(1.0f));
    scene.path_space->load(*right_wall, e8util::mat44_scale(1.0f));
    scene.path_space->load(*back_wall, e8util::mat44_scale(1.0f));
    scene.path_space->load(*ceiling, e8util::mat44_scale(1.0f));
    scene.path_space->load(*floor, e8util::mat44_scale(1.0f));
    scene.path_space->load(*left_sphere, e8util::mat44_scale(1.0f));
    scene.path_space->load(*right_sphere, e8util::mat44_scale(1.0f));
    scene.path_space->load(*light_geo, e8util::mat44_scale(1.0f));
    scene.path_space->commit();

    std::shared_ptr<e8::if_light> light = std::make_shared<e8::area_light>(
        "light", light_geo, e8util::vec3{0.911f, 0.660f, 0.345f} * 15.0f);

    scene.light_sources->load(*light, e8util::mat44_scale(1.0f));
    scene.light_sources->commit();

    e8util::mat44 trans = e8util::mat44_translate({0.0f, 0.795f, 3.4f});
    e8util::mat44 rot = e8util::mat44_rotate(0.0f, {0, 0, 1});
    scene.camera = std::make_unique<e8::pinhole_camera>("cornell_cam", 0.032f, 0.035f, 4.0f / 3.0f)
                       ->transform(trans * rot);

    return scene;
}

void tst_renderer::pt_render_cornel_balls() {
    cornell_balls scene = cornell_box_path_space();
    e8::pt_image_renderer renderer(
        std::make_unique<e8::pathtracer_factory>(e8::pathtracer_factory::unidirect_lt1,
                                                 e8::pathtracer_factory::options()),
        /*num_threads=*/1);
    for (unsigned k = 0; k < 10; k++) {
        e8::clamp_compositor compositor(/*width=*/800, /*height=*/600);
        renderer.render(&compositor, *scene.path_space, *scene.mats, *scene.light_sources,
                        *scene.camera,
                        /*num_samps=*/1, /*firefly_filter=*/false);

        e8util::vec3 irradiance;
        for (unsigned j = 0; j < compositor.height(); j++) {
            for (unsigned i = 0; i < compositor.width(); i++) {
                QVERIFY2(!std::isnan(compositor(i, j)(0)),
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j))
                             .c_str());
                QVERIFY2(!std::isnan(compositor(i, j)(1)),
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j))
                             .c_str());
                QVERIFY2(!std::isnan(compositor(i, j)(2)),
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j))
                             .c_str());

                QVERIFY2(!std::isinf(compositor(i, j)(0)),
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j))
                             .c_str());
                QVERIFY2(!std::isnan(compositor(i, j)(1)),
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j))
                             .c_str());
                QVERIFY2(!std::isnan(compositor(i, j)(2)),
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j))
                             .c_str());

                QVERIFY2(compositor(i, j)(1) >= 0,
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j) +
                          "," + std::to_string(compositor(i, j)(1)))
                             .c_str());
                QVERIFY2(compositor(i, j)(2) >= 0,
                         (std::to_string(k) + "," + std::to_string(i) + "," + std::to_string(j) +
                          "," + std::to_string(compositor(i, j)(2)))
                             .c_str());
                irradiance += compositor(i, j).cart();
            }
        }

        QVERIFY(irradiance(0) > 5000.0f);
        QVERIFY(irradiance(1) > 5000.0f);
        QVERIFY(irradiance(2) > 5000.0f);
    }
}

QTEST_APPLESS_MAIN(tst_renderer)

#include "tst_renderer.moc"
