#include <cmath>
#include <omp.h>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

#define PI 3.1415926535897932384626433832795f

/*
 * Thread-safe random number generator
 */

struct rng {
    rng() : distrb(0.0, 1.0), engines() {}

    void init(size_t nworkers) {
        std::random_device rd;
        engines.resize(nworkers);
        for (size_t i = 0; i < nworkers; ++i)
            engines[i].seed(rd());
    }

    float operator()() {
        size_t id = static_cast<size_t>(omp_get_thread_num());
        return distrb(engines[id]);
    }

    std::uniform_real_distribution<float> distrb;
    std::vector<std::mt19937> engines;
};

static rng rng;

/*
 * Basic data types
 */

struct vec {
    float x, y, z;

    vec(float x_ = 0, float y_ = 0, float z_ = 0) {
        x = x_;
        y = y_;
        z = z_;
    }

    vec operator+(const vec &b) const { return vec(x + b.x, y + b.y, z + b.z); }
    vec operator-(const vec &b) const { return vec(x - b.x, y - b.y, z - b.z); }
    vec operator*(float b) const { return vec(x * b, y * b, z * b); }

    vec mult(const vec &b) const { return vec(x * b.x, y * b.y, z * b.z); }
    vec div(const vec &b) const { return vec(x / b.x, y / b.y, z / b.z); }
    vec &normalize() { return *this = *this * (1.0f / std::sqrt(x * x + y * y + z * z)); }
    float dot(const vec &b) const { return x * b.x + y * b.y + z * b.z; }
    vec cross(const vec &b) const {
        return vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }
};

struct ray {
    vec o, d;
    ray(vec o_, vec d_) : o(o_), d(d_) {}
};

struct material {
    material() = default;
    virtual ~material() = default;
    virtual vec eval(const vec &n, const vec &o, const vec &i) const = 0;
    virtual void sample(const vec &n, const vec &o, vec &i, float &pdf) const = 0;
};

/*
 * Utility functions
 */

inline float clamp(float x) { return x < 0 ? 0 : x > 1 ? 1 : x; }

inline int to_int(float x) { return static_cast<int>(std::pow(clamp(x), 1.0 / 2.2) * 255 + .5); }

/*
 * Shapes
 */

struct sphere {
    vec p, e;             // position, emitted radiance
    const material &brdf; // BRDF
    float rad;            // radius

    sphere(float rad, vec p, vec e, const material &brdf) : p(p), e(e), brdf(brdf), rad(rad) {}

    float intersect(const ray &r) const { // returns distance, 0 if nohit
        vec op = p - r.o;                 // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        float t, eps = 1e-4f, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
        if (det < 0)
            return 0;
        else
            det = std::sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }
};

/*
 * Sampling functions
 */

inline void create_local_coord(const vec &n, vec &u, vec &v) {
    u = ((std::abs(n.x) > .1f ? vec(0, 1) : vec(1)).cross(n)).normalize();
    v = n.cross(u);
}

/*
 * BRDFs
 */

// Ideal diffuse BRDF
struct lambertian_material : public material {
    lambertian_material(vec kd_) : kd(kd_) {}

    vec eval(const vec & /*n*/, const vec & /*o*/, const vec & /*i*/) const {
        return kd * (1.0f / static_cast<float>(M_PI));
    }

    void sample(const vec &n, const vec & /*o*/, vec &i, float &pdf) const {
        float z = std::sqrt(rng());
        float r = std::sqrt(1.0f - z * z);
        float phi = 2.0f * static_cast<float>(M_PI) * rng();
        float x = r * std::cos(phi);
        float y = r * std::sin(phi);

        vec b1, b2;
        create_local_coord(n, b1, b2);

        i = b1 * x + b2 * y + n * z;
        pdf = i.dot(n) / static_cast<float>(M_PI);
    }

    vec kd;
};

// Ideal reflection.
struct mirror_material : public material {
    mirror_material(vec ks_) : ks(ks_) {}

    vec eval(const vec &n, const vec &o, const vec &i) const {
        vec r = n * 2.0 * n.dot(o) - o;
        if (std::abs(r.dot(i) - 1) < 1e-2f)
            return ks * (1 / n.dot(i));
        else
            return 0;
    }

    void sample(const vec &n, const vec &o, vec &i, float &pdf) const {
        i = n * 2.0 * n.dot(o) - o;
        pdf = 1.0;
    }

    vec ks;
};

// Oren-nayar
struct oren_nayar_material : public material {
    oren_nayar_material(vec kd, double roughness) : kd(kd), roughness(roughness) {}

    vec eval(const vec &n, const vec &o, const vec &i) const {
        const float A = 1.0f - 0.5f * roughness / (roughness + 0.33f);
        const float B = 0.45f * roughness / (roughness + 0.09f);

        float cos_thei = i.dot(n);
        float cos_theo = o.dot(n);
        float cos_alpha, cos_beta;

        if (cos_thei < cos_theo) {
            cos_alpha = cos_thei;
            cos_beta = cos_theo;
        } else {
            cos_alpha = cos_theo;
            cos_beta = cos_thei;
        }

        float sin_alpha = std::sqrt(1.0f - cos_alpha * cos_alpha);
        float sin_beta = std::sqrt(1.0f - cos_beta * cos_beta);
        float tan_beta = sin_beta / cos_beta;

        float cos_theio = cos_alpha * cos_beta + sin_alpha * sin_beta;

        return kd * (1.0f / static_cast<float>(M_PI)) *
               (A + B * std::max(0.0f, cos_theio) * sin_alpha * tan_beta);
    }

    void sample(const vec &n, const vec &o, vec &i, float &pdf) const {
        float z = std::sqrt(rng());
        float r = std::sqrt(1.0f - z * z);
        float phi = 2.0f * static_cast<float>(M_PI) * rng();
        float x = r * std::cos(phi);
        float y = r * std::sin(phi);

        vec b1, b2;
        create_local_coord(n, b1, b2);

        i = b1 * x + b2 * y + n * z;
        pdf = i.dot(n) / static_cast<float>(M_PI);
    }

    vec kd;
    float roughness;
};

// Cook Torrance
struct cook_torr_material : public material {
    cook_torr_material(vec ks, double beta, double ior) : ks(ks), beta(beta), ior(ior) {}

    float fresnel(const vec &i, const vec &h) const {
        float c = i.dot(h);
        float g = std::sqrt(ior * ior - 1.0f + c * c);
        float f = (g - c) / (g + c);
        float d = (c * (g + c) - 1.0f) / (c * (g - c) + 1.0f);
        return 0.5f * f * f * (1.0f + d * d);
    }

    float ggx_distri(const vec &n, const vec &h) const {
        float cos_th = n.dot(h);
        float cos_th2 = cos_th * cos_th;
        float tan_th2 = 1.0f / cos_th2 - 1.0f;
        float b2 = beta * beta;
        float c = b2 + tan_th2;
        return b2 / (PI * cos_th2 * cos_th2 * c * c);
    }

    float ggx_shadow1(const vec &v, const vec &h) const {
        float cos_vh = v.dot(h);
        float tan_tv2 = 1.0f / (cos_vh * cos_vh) - 1.0f;
        float b2 = beta * beta;
        return 2.0f / (1.0f + std::sqrt(1.0f + b2 * tan_tv2));
    }

    float ggx_shadow(const vec &i, const vec &o, const vec &h) const {
        return ggx_shadow1(i, h) * ggx_shadow1(o, h);
    }

    vec eval(const vec &n, const vec &o, const vec &i) const {
        const float bias = 1e-4f;
        vec h = (i + o).normalize();
        float cos_the = std::max(n.dot(i), 0.0f);
        float b_cos_the = cos_the + (cos_the == 0.0f ? bias : 0.0f);

        float F = fresnel(i, h);
        float D = ggx_distri(n, h);
        float G = ggx_shadow(i, o, n);

        float cook = F * D * G * (1.0f / (4.0f * b_cos_the * n.dot(o)));
        return ks * cook;
    }

    void sample(const vec &n, const vec &o, vec &i, float &pdf) const {
        vec b1, b2;
        create_local_coord(n, b1, b2);

        float theta = 2.0f * static_cast<float>(M_PI) * rng();
        float t = rng();
        float phi = std::atan(beta * std::sqrt(t / (1.0f - t)));
        float sin_phi = std::sin(phi);
        float cos_phi = std::cos(phi);
        float x = sin_phi * std::cos(theta);
        float y = sin_phi * std::sin(theta);
        float z = cos_phi;

        vec m = b1 * x + b2 * y + n * z;
        float m_dot_o = std::abs(m.dot(o));
        i = m * m_dot_o * 2.0 - o;

        pdf = ggx_distri(n, m) * std::abs(cos_phi) / (4.0f * m_dot_o);
    }

    vec ks;
    float beta;
    float ior;
};

/*
 * Scene configuration
 */

// Pre-defined BRDFs
/*
const DiffuseBRDF leftWall(Vec(.75,.25,.25)),
rightWall(Vec(.25,.25,.75)),
otherWall(Vec(.75,.75,.75)),
blackSurf(Vec(0.0,0.0,0.0)),
brightSurf(Vec(0.9,0.9,0.9));
*/

const oren_nayar_material leftWall(vec(.75, .25, .25), 0.8), rightWall(vec(.25, .25, .75), 0.8),
    otherWall(vec(.75, .75, .75), 0.8), blackSurf(vec(0.0, 0.0, 0.0), 0.8),
    brightSurf(vec(0.9f, 0.9f, 0.9f), 0.8);

// const MirrorBRDF mirrorSurf(Vec(0.9,0.9,0.9));
const cook_torr_material metalSurf(vec(1.0, 1.0, 1.0), 0.01, 1.8);

// Scene: list of spheres
const sphere spheres[] = {
    sphere(1e5, vec(1e5f + 1, 40.8f, 81.6f), vec(), leftWall),    // Left
    sphere(1e5, vec(-1e5f + 99, 40.8f, 81.6f), vec(), rightWall), // Right
    sphere(1e5, vec(50, 40.8f, 1e5), vec(), otherWall),           // Back
    sphere(1e5, vec(50, 1e5, 81.6f), vec(), otherWall),           // Bottom
    sphere(1e5, vec(50, -1e5f + 81.6f, 81.6f), vec(), otherWall), // Top
    sphere(16.5, vec(27, 16.5, 47), vec(), brightSurf),           // Ball 1
    sphere(16.5, vec(73, 16.5, 78), vec(), metalSurf),            // Ball 2
    sphere(5.0, vec(50, 70.0, 81.6f), vec(50, 50, 50), blackSurf) // Light
};

// Camera position & direction
const ray cam(vec(50, 52, 295.6f), vec(0, -0.042612f, -1).normalize());

/*
 * Global functions
 */

bool intersect(const ray &r, float &t, int &id) {
    float n = sizeof(spheres) / sizeof(sphere), d, inf = t = 1e20f;
    for (int i = int(n); i--;)
        if ((d = spheres[i].intersect(r)) != 0.0f && d < t) {
            t = d;
            id = i;
        }
    return t < inf;
}

/*
 * KEY FUNCTION: radiance estimator
 */

vec received_radiance(const ray &r, int depth, bool flag);

vec direct(const vec &n, const vec &o, const vec &p_hit, const sphere &obj, int id) {
    const sphere &light = spheres[7];

    float u = rng();
    float v = rng();

    float z = 2 * u - 1;
    float x = std::sqrt(1 - z * z) * std::cos(2 * static_cast<float>(M_PI) * v);
    float y = std::sqrt(1 - z * z) * std::sin(2 * static_cast<float>(M_PI) * v);

    vec w = vec(x, y, z);
    vec p = light.p + w * light.rad;

    vec i = p - p_hit;
    float d2 = i.dot(i);
    float d = std::sqrt(d2);

    i = i * (1.0f / d);

    // Occlusion test.
    float t;
    int id2;
    intersect(ray(p_hit, i), t, id2);
    if (t < d - 1e-5f)
        return 0.0f;

    if (id == 7)
        return 0;

    float inv_p = 4 * static_cast<float>(M_PI) * light.rad * light.rad;
    float inv_d2 = 1.0f / d2;

    float cos_the = i.dot(n);
    float cos_phi = i.dot(w * -1.0f);
    return obj.brdf.eval(n, o, i).mult(light.e) * cos_the * cos_phi * inv_d2 * inv_p;
}

vec indirect(const vec &n, const vec &o, const vec &p_hit, const sphere &obj, int depth) {
    vec i;
    float pdf;
    obj.brdf.sample(n, o, i, pdf);
    vec li = received_radiance(ray(p_hit, i), depth + 1, true);

    float cos_the = i.dot(n);
    return obj.brdf.eval(n, o, i).mult(li) * (cos_the / pdf);
}

vec radiance(const vec &n, const vec &o, const vec &p_hit, const sphere &obj, int id) {
    return obj.e + direct(n, o, p_hit, obj, id) + indirect(n, o, p_hit, obj, 2);
}

vec received_radiance(const ray &r, int depth, bool /*flag*/) {
    float t;    // Distance to intersection
    int id = 0; // id of intersected sphere

    if (!intersect(r, t, id))
        return vec();                // if miss, return black
    const sphere &obj = spheres[id]; // the hit object

    vec x = r.o + r.d * t;             // The intersection point
    vec o = (vec() - r.d).normalize(); // The outgoing direction (= -r.d)

    vec n = (x - obj.p).normalize(); // The normal direction
    if (n.dot(o) < 0)
        n = n * -1.0;

    // Base case.
    if (depth == 1)
        return radiance(n, o, x, obj, id);

    // Direct illuminant.
    vec Ld = direct(n, o, x, obj, id);

    // Mutation.
    static const int mutate_depth = 3;
    float p_survive = 0.5;
    if (depth >= mutate_depth) {
        if (rng() >= p_survive)
            return Ld;
    } else
        p_survive = 1;

    // Indirect illuminant.
    vec Li = indirect(n, o, x, obj, depth);
    vec L = Ld + Li * (1 / p_survive);
    return L;
}

float to_lum(const vec &c) { return c.x * 0.299f + c.y * 0.587f + c.z * 0.114f; }

float exposure(const std::vector<vec> &c) {
    float s = 0;
    for (unsigned i = 0; i < c.size(); i++) {
        s += std::log(to_lum(c[i]) + 1e-3f);
    }
    s /= c.size();
    return std::exp(s);
}

void aces_tonemap(std::vector<vec> &c, float exposure) {
    const float A = 2.51f;
    const float B = 0.03f;
    const float C = 2.43f;
    const float D = 0.59f;
    const float E = 0.14f;
    for (unsigned i = 0; i < c.size(); i++) {
        vec color = c[i] * exposure;
        c[i] = color.mult(color * A + vec(B, B, B))
                   .div(color.mult(color * C + vec(D, D, D)) + vec(E, E, E));
    }
}

/*
 * Main function (do not modify)
 */

int main(int argc, char *argv[]) {
    int nworkers = omp_get_num_procs();
    omp_set_num_threads(nworkers);
    rng.init(static_cast<size_t>(nworkers));

    unsigned w = 1024, h = 768, samps = argc == 2 ? atoi(argv[1]) / 4 : 32; // # samples
    vec cx = vec(w * .5135f / h), cy = (cx.cross(cam.d)).normalize() * .5135f;
    std::vector<vec> c(w * h);

#pragma omp parallel for schedule(dynamic, 1)
    for (unsigned y = 0; y < h; y++) {
        for (unsigned x = 0; x < w; x++) {
            const unsigned i = (h - y - 1) * w + x;

            for (unsigned sy = 0; sy < 2; ++sy) {
                for (unsigned sx = 0; sx < 2; ++sx) {
                    vec r;
                    for (unsigned s = 0; s < samps; s++) {
                        float r1 = 2 * rng(),
                              dx = r1 < 1 ? std::sqrt(r1) - 1 : 1 - std::sqrt(2 - r1);
                        float r2 = 2 * rng(),
                              dy = r2 < 1 ? std::sqrt(r2) - 1 : 1 - std::sqrt(2 - r2);
                        vec d = cx * (((sx + .5f + dx) / 2 + x) / w - .5f) +
                                cy * (((sy + .5f + dy) / 2 + y) / h - .5f) + cam.d;
                        r = r +
                            received_radiance(ray(cam.o, d.normalize()), 1, true) * (1.f / samps);
                    }
                    c[i] = c[i] + vec(r.x, r.y, r.z) * .25;
                }
            }
        }
#pragma omp critical
        fprintf(stderr, "\rRendering (%d spp) %6.2f%%", samps * 4, 100. * y / (h - 1));
    }
    fprintf(stderr, "\n");

    // aces_tonemap(c, exposure(c));

    // Write resulting image to a PPM file
    FILE *f = fopen("image.ppm", "w");
    fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
    for (unsigned i = 0; i < w * h; i++) {
        int x = to_int(c[i].x);
        int y = to_int(c[i].y);
        int z = to_int(c[i].z);
        fprintf(f, "%d %d %d ", x, y, z);
    }
    fclose(f);

    return 0;
}
