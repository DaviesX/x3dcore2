#include "src/material.h"
#include <QtTest>

class tst_material : public QObject {
    Q_OBJECT

  public:
    tst_material();
    ~tst_material();

  private slots:
    void cook_torrance_name();
    void cook_torrance_sample_dir();
    void cook_torrance_radiance();

    void oren_nayar_name();
    void oren_nayar_sample_dir();
    void oren_nayar_radiance();
};

tst_material::tst_material() {}

tst_material::~tst_material() {}

void tst_material::cook_torrance_name() {
    e8::cook_torr mat =
        e8::cook_torr("test_cook_torr", e8util::vec3({0.787f, 0.787f, 0.787f}), 0.25f, 2.93f);
    QVERIFY(mat.name() == "test_cook_torr");
}

void tst_material::oren_nayar_name() {
    e8::oren_nayar mat =
        e8::oren_nayar("test_oren_nayar", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    QVERIFY(mat.name() == "test_oren_nayar");
}

void generic_validate_mat_dir(float *p_valid, std::vector<e8util::vec3> *samples,
                              e8::if_material const &mat) {
    samples->clear();

    e8util::vec3 normal{0.0f, 0.0f, 1.0f};
    e8util::vec3 o_ray{1.0f, 1.0f, 1.0f};
    o_ray = o_ray.normalize();
    e8util::rng rng(13);

    unsigned const NUM_SAMPLES = 1000;
    for (unsigned i = 0; i < NUM_SAMPLES; i++) {
        float dens = -1;
        e8util::vec3 i_ray = mat.sample(&rng, &dens, /*uv=*/e8util::vec2(), normal, o_ray);

        QVERIFY(dens >= 0.0f);
        if (!e8util::equals(i_ray, e8util::vec3())) {
            QVERIFY(e8util::equals(i_ray.norm(), 1.0f));
            QVERIFY(i_ray.inner(normal) >= 0.0f);
            samples->push_back(i_ray);
        }
    }

    *p_valid = static_cast<float>(samples->size()) / NUM_SAMPLES;
}

void generic_validate_mat_radiance(e8::if_material const &mat) {
    float p_valid;
    std::vector<e8util::vec3> samples;
    generic_validate_mat_dir(&p_valid, &samples, mat);

    for (e8util::vec3 const &i_ray : samples) {
        e8util::vec3 normal{0.0f, 0.0f, 1.0f};
        e8util::vec3 o_ray{1.0f, 1.0f, 1.0f};
        e8util::vec3 radiance = mat.eval(/*uv=*/e8util::vec2(), normal, o_ray, i_ray);
        QVERIFY(radiance(0) >= 0.0f && radiance(1) >= 0.0f && radiance(2) >= 0.0f);
    }
}

void tst_material::cook_torrance_sample_dir() {
    e8::cook_torr mat =
        e8::cook_torr("test_cook_torr", e8util::vec3({0.787f, 0.787f, 0.787f}), 0.2f, 2.93f);
    float p_valid;
    std::vector<e8util::vec3> samples;
    generic_validate_mat_dir(&p_valid, &samples, mat);
    QVERIFY(p_valid >= .8f);
}

void tst_material::oren_nayar_sample_dir() {
    e8::oren_nayar mat =
        e8::oren_nayar("test_oren_nayar", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    float p_valid;
    std::vector<e8util::vec3> samples;
    generic_validate_mat_dir(&p_valid, &samples, mat);
    QVERIFY(p_valid == 1.0f);
}

void tst_material::cook_torrance_radiance() {
    e8::cook_torr mat =
        e8::cook_torr("test_cook_torr", e8util::vec3({0.787f, 0.787f, 0.787f}), 0.25f, 2.93f);
    generic_validate_mat_radiance(mat);
}

void tst_material::oren_nayar_radiance() {
    e8::oren_nayar mat =
        e8::oren_nayar("test_oren_nayar", e8util::vec3({0.725f, 0.710f, 0.680f}), 0.078f);
    generic_validate_mat_radiance(mat);
}

QTEST_APPLESS_MAIN(tst_material)

#include "tst_material.moc"
