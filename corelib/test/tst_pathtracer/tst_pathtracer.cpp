#include "src/geometry.h"
#include "src/lightsources.h"
#include "src/pathspace.h"
#include "src/pathtracer.h"
#include "src/resource.h"
#include <QString>
#include <QtTest>
#include <iostream>
#include <memory>

class tst_pathtracer : public QObject {
    Q_OBJECT

  public:
    tst_pathtracer() = default;

  private Q_SLOTS:
    void unidirect_tracer();
};

struct sphere_scene {
    sphere_scene() {
        std::shared_ptr<e8::if_material> material =
            std::make_shared<e8::oren_nayar>("material", albedo, /*roughness=*/0.0f);

        std::shared_ptr<e8::if_geometry> sphere = std::make_shared<e8::uv_sphere>(
            /*name=*/"sphere", /*o=*/e8util::vec3{0.0f, 0.0f, 0.0f}, /*r=*/10.0f,
            /*res=*/30, /*flip_normal=*/true);
        sphere->add_child(material);

        std::shared_ptr<e8::if_light> light =
            std::make_shared<e8::area_light>("light", sphere, light_rad);
        light->add_child(sphere);

        path_space = std::make_unique<e8::bvh_path_space_layout>();
        path_space->load(*sphere, e8util::mat44_scale(1.0f));
        path_space->commit();

        light_sources = std::make_unique<e8::basic_light_sources>();
        light_sources->load(*light, e8util::mat44_scale(1.0f));
        light_sources->commit();
    }

    e8util::vec3 albedo = 0.7f;
    e8util::vec3 light_rad = 1.0f;

    std::unique_ptr<e8::if_path_space> path_space;
    std::unique_ptr<e8::if_light_sources> light_sources;
};

void inner_sphere_validation(e8::if_path_tracer const &tracer, unsigned num_samps_per_dir) {
    sphere_scene scene;
    e8util::rng rn(13);
    float sum_x = 0;
    unsigned const k = 10;
    for (unsigned i = 0; i < k; i++) {
        e8util::vec3 dir = e8util::vec3_sphere_sample(rn.draw(), rn.draw());
        e8util::ray r(e8util::vec3{0, 0, 0}, dir);

        e8::if_path_tracer::first_hits hits = e8::if_path_tracer::compute_first_hit(
            std::vector<e8util::ray>{r}, *scene.path_space, *scene.light_sources);

        for (unsigned j = 0; j < num_samps_per_dir; j++) {
            std::vector<e8util::vec3> estimate = tracer.sample(
                rn, std::vector<e8util::ray>{r}, hits, *scene.path_space, *scene.light_sources);
            QVERIFY(estimate.size() == 1);
            QVERIFY2(estimate[0](0) > 0 && estimate[0](1) > 0 && estimate[0](2) > 0,
                     ("At " + std::to_string(i) + "|" + std::to_string(j)).c_str());
            sum_x += estimate[0].sum();
        }
    }

    float n = k * num_samps_per_dir;
    float mu = sum_x / n;
    float exp_x = (scene.light_rad / (e8util::vec3(1) - scene.albedo)).sum();
    QVERIFY2(std::sqrt((mu - exp_x) * (mu - exp_x)) < 1,
             ("mu=" + std::to_string(mu) + "|exp{x}=" + std::to_string(exp_x)).c_str());
}

void tst_pathtracer::unidirect_tracer() {
    inner_sphere_validation(e8::unidirect_path_tracer(), /*num_samps_per_dir=*/256);
}

QTEST_APPLESS_MAIN(tst_pathtracer)

#include "tst_pathtracer.moc"
