#include <cmath>
#include <omp.h>
#include <random>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#define PI 3.1415926535897932384626433832795

/*
 * Thread-safe random number generator
 */

struct RNG {
    RNG() : distrb(0.0, 1.0), engines() {}

    void init(int nworkers) {
        std::random_device rd;
        engines.resize(nworkers);
        for (int i = 0; i < nworkers; ++i)
            engines[i].seed(rd());
    }

    double operator()() {
        int id = omp_get_thread_num();
        return distrb(engines[id]);
    }

    std::uniform_real_distribution<double> distrb;
    std::vector<std::mt19937> engines;
} rng;

/*
 * Basic data types
 */

struct Vec {
    double x, y, z;

    Vec(double x_ = 0, double y_ = 0, double z_ = 0) {
        x = x_;
        y = y_;
        z = z_;
    }

    Vec operator+(const Vec &b) const { return Vec(x + b.x, y + b.y, z + b.z); }
    Vec operator-(const Vec &b) const { return Vec(x - b.x, y - b.y, z - b.z); }
    Vec operator*(double b) const { return Vec(x * b, y * b, z * b); }

    Vec mult(const Vec &b) const { return Vec(x * b.x, y * b.y, z * b.z); }
    Vec div(const Vec &b) const { return Vec(x / b.x, y / b.y, z / b.z); }
    Vec &normalize() { return *this = *this * (1.0 / std::sqrt(x * x + y * y + z * z)); }
    double dot(const Vec &b) const { return x * b.x + y * b.y + z * b.z; }
    Vec cross(const Vec &b) const {
        return Vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }
};

struct Ray {
    Vec o, d;
    Ray(Vec o_, Vec d_) : o(o_), d(d_) {}
};

struct BRDF {
    virtual Vec eval(const Vec &n, const Vec &o, const Vec &i) const = 0;
    virtual void sample(const Vec &n, const Vec &o, Vec &i, double &pdf) const = 0;
};

/*
 * Utility functions
 */

inline double clamp(double x) { return x < 0 ? 0 : x > 1 ? 1 : x; }

inline int toInt(double x) { return static_cast<int>(std::pow(clamp(x), 1.0 / 2.2) * 255 + .5); }

/*
 * Shapes
 */

struct Sphere {
    Vec p, e;         // position, emitted radiance
    double rad;       // radius
    const BRDF &brdf; // BRDF

    Sphere(double rad_, Vec p_, Vec e_, const BRDF &brdf_) : rad(rad_), p(p_), e(e_), brdf(brdf_) {}

    double intersect(const Ray &r) const { // returns distance, 0 if nohit
        Vec op = p - r.o;                  // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double t, eps = 1e-4, b = op.dot(r.d), det = b * b - op.dot(op) + rad * rad;
        if (det < 0)
            return 0;
        else
            det = sqrt(det);
        return (t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0);
    }
};

/*
 * Sampling functions
 */

inline void createLocalCoord(const Vec &n, Vec &u, Vec &v) {
    u = ((std::abs(n.x) > .1 ? Vec(0, 1) : Vec(1)).cross(n)).normalize();
    v = n.cross(u);
}

/*
 * BRDFs
 */

// Ideal diffuse BRDF
struct DiffuseBRDF : public BRDF {
    DiffuseBRDF(Vec kd_) : kd(kd_) {}

    Vec eval(const Vec &n, const Vec &o, const Vec &i) const { return kd * (1.0 / PI); }

    void sample(const Vec &n, const Vec &o, Vec &i, double &pdf) const {
        double z = std::sqrt(rng());
        double r = std::sqrt(1.0 - z * z);
        double phi = 2.0 * M_PI * rng();
        double x = r * std::cos(phi);
        double y = r * std::sin(phi);

        Vec b1, b2;
        createLocalCoord(n, b1, b2);

        i = b1 * x + b2 * y + n * z;
        pdf = i.dot(n) / M_PI;
    }

    Vec kd;
};

// Ideal reflection.
struct MirrorBRDF : public BRDF {
    MirrorBRDF(Vec ks_) : ks(ks_) {}

    Vec eval(const Vec &n, const Vec &o, const Vec &i) const {
        Vec r = n * 2.0 * n.dot(o) - o;
        if (std::abs(r.dot(i) - 1) < 1e-2)
            return ks * (1 / n.dot(i));
        else
            return 0;
    }

    void sample(const Vec &n, const Vec &o, Vec &i, double &pdf) const {
        i = n * 2.0 * n.dot(o) - o;
        pdf = 1.0;
    }

    Vec ks;
};

// Oren-nayar
struct OrenNayarBRDF : public BRDF {
    OrenNayarBRDF(Vec kd, double roughness) : kd(kd), roughness(roughness) {}

    Vec eval(const Vec &n, const Vec &o, const Vec &i) const {
        const float A = 1 - 0.5 * roughness / (roughness + 0.33);
        const float B = 0.45 * roughness / (roughness + 0.09);

        double cos_thei = i.dot(n);
        double cos_theo = o.dot(n);
        float cos_alpha, cos_beta;

        if (cos_thei < cos_theo) {
            cos_alpha = cos_thei;
            cos_beta = cos_theo;
        } else {
            cos_alpha = cos_theo;
            cos_beta = cos_thei;
        }

        double sin_alpha = std::sqrt(1.0 - cos_alpha * cos_alpha);
        double sin_beta = std::sqrt(1.0 - cos_beta * cos_beta);
        double tan_beta = sin_beta / cos_beta;

        double cos_theio = cos_alpha * cos_beta + sin_alpha * sin_beta;

        return kd * (1.0 / M_PI) * (A + B * std::max(0.0, cos_theio) * sin_alpha * tan_beta);
    }

    void sample(const Vec &n, const Vec &o, Vec &i, double &pdf) const {
        double z = sqrt(rng());
        double r = std::sqrt(1.0 - z * z);
        double phi = 2.0 * M_PI * rng();
        double x = r * std::cos(phi);
        double y = r * std::sin(phi);

        Vec b1, b2;
        createLocalCoord(n, b1, b2);

        i = b1 * x + b2 * y + n * z;
        pdf = i.dot(n) / M_PI;
    }

    Vec kd;
    double roughness;
};

// Cook Torrance
struct CookTorrBRDF : public BRDF {
    CookTorrBRDF(Vec ks, double beta, double ior) : ks(ks), beta(beta), ior(ior) {}

    double fresnel(const Vec &i, const Vec &h) const {
        double c = i.dot(h);
        double g = std::sqrt(ior * ior - 1.0 + c * c);
        double f = (g - c) / (g + c);
        double d = (c * (g + c) - 1.0) / (c * (g - c) + 1.0);
        return 0.5 * f * f * (1.0 + d * d);
    }

    double ggx_distri(const Vec &n, const Vec &h) const {
        double cos_th = n.dot(h);
        double cos_th2 = cos_th * cos_th;
        double tan_th2 = 1.0 / cos_th2 - 1.0;
        double b2 = beta * beta;
        double c = b2 + tan_th2;
        return b2 / (PI * cos_th2 * cos_th2 * c * c);
    }

    double ggx_shadow1(const Vec &v, const Vec &h) const {
        double cos_vh = v.dot(h);
        double tan_tv2 = 1.0 / (cos_vh * cos_vh) - 1.0;
        double b2 = beta * beta;
        return 2.0 / (1.0 + sqrt(1.0 + b2 * tan_tv2));
    }

    double ggx_shadow(const Vec &i, const Vec &o, const Vec &h) const {
        return ggx_shadow1(i, h) * ggx_shadow1(o, h);
    }

    Vec eval(const Vec &n, const Vec &o, const Vec &i) const {
        const double bias = 1e-4f;
        Vec h = (i + o).normalize();
        double cos_the = std::max(n.dot(i), 0.0);
        double b_cos_the = cos_the + (cos_the == 0.0 ? bias : 0.0);

        double F = fresnel(i, h);
        double D = ggx_distri(n, h);
        double G = ggx_shadow(i, o, n);

        double cook = F * D * G * (1.0 / (4.0 * b_cos_the * n.dot(o)));
        return ks * cook;
    }

    void sample(const Vec &n, const Vec &o, Vec &i, double &pdf) const {
        Vec b1, b2;
        createLocalCoord(n, b1, b2);

        double theta = 2.0 * M_PI * rng();
        double t = rng();
        double phi = std::atan(beta * std::sqrt(t / (1.0 - t)));
        double sin_phi = std::sin(phi);
        double cos_phi = std::cos(phi);
        double x = sin_phi * std::cos(theta);
        double y = sin_phi * std::sin(theta);
        double z = cos_phi;

        Vec m = b1 * x + b2 * y + n * z;
        double m_dot_o = std::abs(m.dot(o));
        i = m * m_dot_o * 2.0 - o;

        pdf = ggx_distri(n, m) * std::abs(cos_phi) / (4.0 * m_dot_o);
    }

    Vec ks;
    double beta;
    double ior;
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

const OrenNayarBRDF leftWall(Vec(.75, .25, .25), 0.8), rightWall(Vec(.25, .25, .75), 0.8),
    otherWall(Vec(.75, .75, .75), 0.8), blackSurf(Vec(0.0, 0.0, 0.0), 0.8),
    brightSurf(Vec(0.9, 0.9, 0.9), 0.8);

// const MirrorBRDF mirrorSurf(Vec(0.9,0.9,0.9));
const CookTorrBRDF metalSurf(Vec(1.0, 1.0, 1.0), 0.01, 1.8);

// Scene: list of spheres
const Sphere spheres[] = {
    Sphere(1e5, Vec(1e5 + 1, 40.8, 81.6), Vec(), leftWall),      // Left
    Sphere(1e5, Vec(-1e5 + 99, 40.8, 81.6), Vec(), rightWall),   // Right
    Sphere(1e5, Vec(50, 40.8, 1e5), Vec(), otherWall),           // Back
    Sphere(1e5, Vec(50, 1e5, 81.6), Vec(), otherWall),           // Bottom
    Sphere(1e5, Vec(50, -1e5 + 81.6, 81.6), Vec(), otherWall),   // Top
    Sphere(16.5, Vec(27, 16.5, 47), Vec(), brightSurf),          // Ball 1
    Sphere(16.5, Vec(73, 16.5, 78), Vec(), metalSurf),           // Ball 2
    Sphere(5.0, Vec(50, 70.0, 81.6), Vec(50, 50, 50), blackSurf) // Light
};

// Camera position & direction
const Ray cam(Vec(50, 52, 295.6), Vec(0, -0.042612, -1).normalize());

/*
 * Global functions
 */

bool intersect(const Ray &r, double &t, int &id) {
    double n = sizeof(spheres) / sizeof(Sphere), d, inf = t = 1e20;
    for (int i = int(n); i--;)
        if ((d = spheres[i].intersect(r)) && d < t) {
            t = d;
            id = i;
        }
    return t < inf;
}

/*
 * KEY FUNCTION: radiance estimator
 */

Vec receivedRadiance(const Ray &r, int depth, bool flag);

Vec direct(const Vec &n, const Vec &o, const Vec &p_hit, const Sphere &obj, int id) {
    const Sphere &light = spheres[7];

    double u = rng();
    double v = rng();

    double z = 2 * u - 1;
    double x = std::sqrt(1 - z * z) * std::cos(2 * M_PI * v);
    double y = std::sqrt(1 - z * z) * std::sin(2 * M_PI * v);

    Vec w = Vec(x, y, z);
    Vec p = light.p + w * light.rad;

    Vec i = p - p_hit;
    double d2 = i.dot(i);
    double d = std::sqrt(d2);

    i = i * (1.0 / d);

    // Occlusion test.
    double t;
    int id2;
    intersect(Ray(p_hit, i), t, id2);
    if (t < d - 1e-5)
        return 0.0;

    if (id == 7)
        return 0;

    double inv_p = 4 * M_PI * light.rad * light.rad;
    double inv_d2 = 1.0 / d2;

    double cos_the = i.dot(n);
    double cos_phi = i.dot(w * -1.0);
    return obj.brdf.eval(n, o, i).mult(light.e) * cos_the * cos_phi * inv_d2 * inv_p;
}

Vec indirect(const Vec &n, const Vec &o, const Vec &p_hit, const Sphere &obj, int depth) {
    Vec i;
    double pdf;
    obj.brdf.sample(n, o, i, pdf);
    Vec li = receivedRadiance(Ray(p_hit, i), depth + 1, true);

    double cos_the = i.dot(n);
    return obj.brdf.eval(n, o, i).mult(li) * (cos_the / pdf);
}

Vec radiance(const Vec &n, const Vec &o, const Vec &p_hit, const Sphere &obj, int id) {
    return obj.e + direct(n, o, p_hit, obj, id) + indirect(n, o, p_hit, obj, 2);
}

Vec receivedRadiance(const Ray &r, int depth, bool flag) {
    double t;   // Distance to intersection
    int id = 0; // id of intersected sphere

    if (!intersect(r, t, id))
        return Vec();                // if miss, return black
    const Sphere &obj = spheres[id]; // the hit object

    Vec x = r.o + r.d * t;             // The intersection point
    Vec o = (Vec() - r.d).normalize(); // The outgoing direction (= -r.d)

    Vec n = (x - obj.p).normalize(); // The normal direction
    if (n.dot(o) < 0)
        n = n * -1.0;

    // Base case.
    if (depth == 1)
        return radiance(n, o, x, obj, id);

    // Direct illuminant.
    Vec Ld = direct(n, o, x, obj, id);

    // Mutation.
    static const int mutate_depth = 3;
    double p_survive = 0.5;
    if (depth >= mutate_depth) {
        if (rng() >= p_survive)
            return Ld;
    } else
        p_survive = 1;

    // Indirect illuminant.
    Vec Li = indirect(n, o, x, obj, depth);
    Vec L = Ld + Li * (1 / p_survive);
    return L;
}

double to_lum(const Vec &c) { return c.x * 0.299 + c.y * 0.587 + c.z * 0.114; }

double exposure(const std::vector<Vec> &c) {
    double s = 0;
    for (unsigned i = 0; i < c.size(); i++) {
        s += std::log(to_lum(c[i]) + 1e-3);
    }
    s /= c.size();
    return std::exp(s);
}

void aces_tonemap(std::vector<Vec> &c, double exposure) {
    const double A = 2.51;
    const double B = 0.03;
    const double C = 2.43;
    const double D = 0.59;
    const double E = 0.14;
    for (unsigned i = 0; i < c.size(); i++) {
        Vec color = c[i] * exposure;
        c[i] = color.mult(color * A + Vec(B, B, B))
                   .div(color.mult(color * C + Vec(D, D, D)) + Vec(E, E, E));
    }
}

/*
 * Main function (do not modify)
 */

int main(int argc, char *argv[]) {
    int nworkers = omp_get_num_procs();
    omp_set_num_threads(nworkers);
    rng.init(nworkers);

    int w = 1024, h = 768, samps = argc == 2 ? atoi(argv[1]) / 4 : 32; // # samples
    Vec cx = Vec(w * .5135 / h), cy = (cx.cross(cam.d)).normalize() * .5135;
    std::vector<Vec> c(w * h);

#pragma omp parallel for schedule(dynamic, 1)
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            const int i = (h - y - 1) * w + x;

            for (int sy = 0; sy < 2; ++sy) {
                for (int sx = 0; sx < 2; ++sx) {
                    Vec r;
                    for (int s = 0; s < samps; s++) {
                        double r1 = 2 * rng(), dx = r1 < 1 ? sqrt(r1) - 1 : 1 - sqrt(2 - r1);
                        double r2 = 2 * rng(), dy = r2 < 1 ? sqrt(r2) - 1 : 1 - sqrt(2 - r2);
                        Vec d = cx * (((sx + .5 + dx) / 2 + x) / w - .5) +
                                cy * (((sy + .5 + dy) / 2 + y) / h - .5) + cam.d;
                        r = r + receivedRadiance(Ray(cam.o, d.normalize()), 1, true) * (1. / samps);
                    }
                    c[i] = c[i] + Vec(r.x, r.y, r.z) * .25;
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
    for (int i = 0; i < w * h; i++) {
        int x = toInt(c[i].x);
        int y = toInt(c[i].y);
        int z = toInt(c[i].z);
        fprintf(f, "%d %d %d ", x, y, z);
    }
    fclose(f);

    return 0;
}
