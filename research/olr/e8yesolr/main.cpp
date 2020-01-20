#include <algorithm>
#include <cmath>
#include <complex>
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
        engines.resize(nworkers);
        size_t s = 23;
        for (size_t i = 0; i < nworkers; ++i) {
            s = (s * 33) ^ (i + 1);
            engines[i].seed(s);
        }
    }

    double operator()() {
        size_t id = static_cast<size_t>(omp_get_thread_num());
        return distrb(engines[id]);
    }

    std::uniform_real_distribution<double> distrb;
    std::vector<std::mt19937> engines;
};

static rng rng;

/*
 * Basic data types
 */

struct vec {
    double x, y, z;

    vec(double x_ = 0, double y_ = 0, double z_ = 0) {
        x = x_;
        y = y_;
        z = z_;
    }

    vec operator+(const vec &b) const { return vec(x + b.x, y + b.y, z + b.z); }
    vec operator-(const vec &b) const { return vec(x - b.x, y - b.y, z - b.z); }
    vec operator*(double b) const { return vec(x * b, y * b, z * b); }

    vec mult(const vec &b) const { return vec(x * b.x, y * b.y, z * b.z); }
    vec div(const vec &b) const { return vec(x / b.x, y / b.y, z / b.z); }
    vec &normalize() { return *this = *this * (1.0 / std::sqrt(x * x + y * y + z * z)); }
    double dot(const vec &b) const { return x * b.x + y * b.y + z * b.z; }
    vec cross(const vec &b) const {
        return vec(y * b.z - z * b.y, z * b.x - x * b.z, x * b.y - y * b.x);
    }
};

struct spectrum {
    float r, g, b;

    spectrum(float r = 0, float g = 0, float b = 0) : r(r), g(g), b(b) {}

    spectrum operator+(const spectrum &other) const {
        return spectrum(r + other.r, g + other.g, b + other.b);
    }
    spectrum operator-(const spectrum &other) const {
        return spectrum(r - other.r, g - other.g, b - other.b);
    }
    spectrum operator*(float scale) const { return spectrum(scale * r, scale * g, scale * b); }

    spectrum mult(const spectrum &other) const {
        return spectrum(r * other.r, g * other.g, b * other.b);
    }

    bool black() const { return r == 0 && g == 0 && b == 0; }
};

struct ray {
    vec o, d;
    ray(vec const &o, vec const &d) : o(o), d(d) {}
};

struct material {
    material() = default;
    virtual ~material();
    virtual spectrum eval(const vec &n, const vec &o, const vec &i) const = 0;
    virtual void sample(const vec &n, const vec &o, vec &i, double &pdf) const = 0;
};

material::~material() {}

/*
 * Utility functions
 */

inline float clamp(float x) { return x < 0 ? 0 : x > 1 ? 1 : x; }

inline int to_int(float x) { return static_cast<int>(std::pow(clamp(x), 1.0f / 2.2f) * 255 + .5f); }

/*
 * Shapes
 */

struct sphere {
    vec p;                // position
    const material &brdf; // BRDF
    double rad;           // radius
    spectrum e;           // emitted radiance
    unsigned padding_ = 0;

    sphere(double rad, vec p, spectrum e, const material &brdf)
        : p(p), brdf(brdf), rad(rad), e(e) {}

    double intersect(const ray &r) const { // returns distance, 0 if nohit
        vec op = p - r.o;                  // Solve t^2*d.d + 2*t*(o-p).d + (o-p).(o-p)-R^2 = 0
        double const eps = 1e-1;
        double b = static_cast<double>(op.dot(r.d));
        double det = b * b - static_cast<double>(op.dot(op)) + rad * rad;
        if (det < 0)
            return 0;
        else
            det = std::sqrt(det);
        double t;
        return static_cast<double>((t = b - det) > eps ? t : ((t = b + det) > eps ? t : 0));
    }
};

/*
 * Sampling functions
 */

inline void create_local_coord(const vec &n, vec &u, vec &v) {
    u = ((std::abs(n.x) > .1 ? vec(0, 1) : vec(1)).cross(n)).normalize();
    v = n.cross(u);
}

/*
 * BRDFs
 */

// Ideal diffuse BRDF
struct lambertian_material : public material {
    lambertian_material(spectrum const &kd_) : kd(kd_) {}
    ~lambertian_material() override;

    spectrum eval(const vec & /*n*/, const vec & /*o*/, const vec & /*i*/) const override {
        return kd * static_cast<float>(1.0 / M_PI);
    }

    void sample(const vec &n, const vec & /*o*/, vec &i, double &pdf) const override {
        double z = std::sqrt(rng());
        double r = std::sqrt(1.0 - z * z);
        double phi = 2.0 * M_PI * rng();
        double x = r * std::cos(phi);
        double y = r * std::sin(phi);

        vec b1, b2;
        create_local_coord(n, b1, b2);

        i = b1 * x + b2 * y + n * z;
        pdf = i.dot(n) / M_PI;
    }

    spectrum kd;
    unsigned padding_ = 0;
};

lambertian_material::~lambertian_material() {}

// Ideal reflection.
struct mirror_material : public material {
    mirror_material(spectrum const &ks_) : ks(ks_) {}
    ~mirror_material() override;

    spectrum eval(const vec &n, const vec &o, const vec &i) const override {
        vec r = n * 2.0 * n.dot(o) - o;
        if (std::abs(r.dot(i) - 1) < 1e-2)
            return ks * static_cast<float>(1 / n.dot(i));
        else
            return 0;
    }

    void sample(const vec &n, const vec &o, vec &i, double &pdf) const override {
        i = n * 2.0 * n.dot(o) - o;
        pdf = 1.0;
    }

    spectrum ks;
    unsigned padding_;
};

mirror_material::~mirror_material() {}

// Oren-nayar
struct oren_nayar_material : public material {
    oren_nayar_material(spectrum const &kd, float roughness) : kd(kd), roughness(roughness) {}
    ~oren_nayar_material() override;

    spectrum eval(const vec &n, const vec &o, const vec &i) const override {
        const float A = 1.0f - 0.5f * roughness / (roughness + 0.33f);
        const float B = 0.45f * roughness / (roughness + 0.09f);

        float cos_thei = static_cast<float>(i.dot(n));
        float cos_theo = static_cast<float>(o.dot(n));
        cos_thei = std::clamp(cos_thei, 0.0f, 1.0f);
        cos_theo = std::clamp(cos_theo, 0.0f, 1.0f);
        if (std::abs(cos_thei) < 1e-4f || std::abs(cos_theo) < 1e-4f) {
            // No radiance can be emitted.
            return spectrum();
        }

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

        return kd * static_cast<float>(1.0 / M_PI) *
               (A + B * std::max(0.0f, cos_theio) * sin_alpha * tan_beta);
    }

    void sample(const vec &n, const vec & /*o*/, vec &i, double &pdf) const override {
        double z = std::sqrt(rng());
        double r = std::sqrt(1.0 - z * z);
        double phi = 2.0 * M_PI * rng();
        double x = r * std::cos(phi);
        double y = r * std::sin(phi);

        vec b1, b2;
        create_local_coord(n, b1, b2);

        i = b1 * x + b2 * y + n * z;
        pdf = i.dot(n) / M_PI;
    }

    spectrum kd;
    float roughness;
};

oren_nayar_material::~oren_nayar_material() {}

// Cook Torrance
struct cook_torr_material : public material {
    cook_torr_material(spectrum const &ks, float beta, std::complex<float> const &ior)
        : ks(ks), beta(beta), ior(ior) {}
    ~cook_torr_material() override;

    float fresnel(vec const &i, vec const &h) const {
        float cos_th_flip = 1 - static_cast<float>(i.dot(h));
        float cos_th_flip2 = cos_th_flip * cos_th_flip;
        float cos_th_flip4 = cos_th_flip2 * cos_th_flip2;
        float cos_th_flip5 = cos_th_flip4 * cos_th_flip;
        float a = (ior.real() - 1) * (ior.real() - 1) + 4 * ior.real() * cos_th_flip5 +
                  ior.imag() * ior.imag();
        float b = (ior.real() + 1) * (ior.real() + 1) + ior.imag() * ior.imag();
        return a / b;
    }

    float ggx_distri(const vec &n, const vec &h) const {
        float cos_th = static_cast<float>(n.dot(h));
        float cos_th2 = cos_th * cos_th;
        float tan_th2 = 1.0f / cos_th2 - 1.0f;
        float b2 = beta * beta;
        float c = b2 + tan_th2;
        return b2 / (static_cast<float>(M_PI) * cos_th2 * cos_th2 * c * c);
    }

    float ggx_shadow1(const vec &v, const vec &h) const {
        float cos_vh = static_cast<float>(v.dot(h));
        float tan_tv2 = 1.0f / (cos_vh * cos_vh) - 1;
        float b2 = beta * beta;
        return 2.0f / (1 + std::sqrt(1 + b2 * tan_tv2));
    }

    float ggx_shadow(const vec &i, const vec &o, const vec &h) const {
        return ggx_shadow1(i, h) * ggx_shadow1(o, h);
    }

    spectrum eval(const vec &n, const vec &o, const vec &i) const override {
        const float bias = 1e-4f;
        vec h = (i + o).normalize();
        float cos_the = static_cast<float>(std::max(n.dot(i), 0.0));
        float b_cos_the = cos_the + (std::abs(cos_the) < bias ? bias : 0);
        if (std::abs(cos_the) < 1e-4f || std::abs(b_cos_the) < 1e-4f ||
            std::abs(static_cast<float>(h.dot(h))) < 1e-4f) {
            return spectrum();
        }

        float F = fresnel(i, h);
        float D = ggx_distri(n, h);
        float G = ggx_shadow(i, o, n);

        float cook = F * D * G * (1.0f / (4 * b_cos_the * static_cast<float>(n.dot(o))));
        return ks * cook;
    }

    void sample(const vec &n, const vec &o, vec &i, double &pdf) const override {
        vec b1, b2;
        create_local_coord(n, b1, b2);

        double theta = 2.0 * M_PI * rng();
        double t = rng();
        double phi = std::atan(static_cast<double>(beta) * std::sqrt(t / (1.0 - t)));
        double sin_phi = std::sin(phi);
        double cos_phi = std::cos(phi);
        double x = sin_phi * std::cos(theta);
        double y = sin_phi * std::sin(theta);
        double z = cos_phi;

        vec m = b1 * x + b2 * y + n * z;
        double m_dot_o = std::abs(m.dot(o));
        i = m * m_dot_o * 2.0 - o;
        if (i.dot(n) < 0) {
            pdf = 0;
        }

        pdf = static_cast<double>(ggx_distri(n, m)) * std::abs(cos_phi) / (4.0 * m_dot_o);
    }

    spectrum ks;
    float beta;
    std::complex<float> ior;
};

cook_torr_material::~cook_torr_material() {}

/*
 * Scene configuration
 */
const oren_nayar_material left_wall(spectrum(.75, .25, .25), 0.8f);
const oren_nayar_material right_wall(spectrum(.25, .25, .75), 0.8f);
const oren_nayar_material other_wall(spectrum(.75, .75, .75), 0.8f);
const oren_nayar_material black_surf(spectrum(0.0, 0.0, 0.0), 0.8f);
const oren_nayar_material bright_surf(spectrum(0.9f, 0.9f, 0.9f), 0.8f);
const cook_torr_material metal_surf(spectrum(.9f, .9f, .9f), 0.01f,
                                    std::complex<float>(2.93f, 3.0));

// Scene: list of spheres
const sphere spheres[] = {
    sphere(1e5, vec(1e5 + 1, 40.8, 81.6), spectrum(), left_wall),      // Left
    sphere(1e5, vec(-1e5 + 99, 40.8, 81.6), spectrum(), right_wall),   // Right
    sphere(1e5, vec(50, 40.8, 1e5), spectrum(), other_wall),           // Back
    sphere(1e5, vec(50, 1e5, 81.6), spectrum(), other_wall),           // Bottom
    sphere(1e5, vec(50, -1e5 + 81.6, 81.6), spectrum(), other_wall),   // Top
    sphere(16.5, vec(27, 16.5, 47), spectrum(), bright_surf),          // Ball 1
    sphere(16.5, vec(73, 16.5, 78), spectrum(), metal_surf),           // Ball 2
    sphere(5.0, vec(50, 70.0, 81.6), spectrum(50, 50, 50), black_surf) // Light
};

// Camera position & direction
const ray cam(vec(50, 52, 295.6), vec(0, -0.042612, -1).normalize());

/*
 * Global functions
 */

bool intersect(const ray &r, double &t, int &id) {
    double n = sizeof(spheres) / sizeof(sphere), d, inf = t = 1e20;
    for (int i = int(n); i--;)
        if ((d = spheres[i].intersect(r)) != 0.0 && d < t) {
            t = d;
            id = i;
        }
    return t < inf;
}

/*
 * KEY FUNCTION: radiance estimator
 */
spectrum pt_received_radiance(const ray &r, int depth);

spectrum direct(const vec &n, const vec &o, const vec &p_hit, const sphere &obj, int id) {
    const sphere &light = spheres[7];

    double u = rng();
    double v = rng();

    double z = 2 * u - 1;
    double x = std::sqrt(1 - z * z) * std::cos(2 * M_PI * v);
    double y = std::sqrt(1 - z * z) * std::sin(2 * M_PI * v);

    vec w = vec(x, y, z);
    vec p = light.p + w * light.rad;

    vec i = p - p_hit;
    double d2 = i.dot(i);
    double d = std::sqrt(d2);

    i = i * (1.0 / d);

    // Occlusion test.
    double t;
    int id2;
    intersect(ray(p_hit, i), t, id2);
    if (t < d - 1e-5)
        return 0.0;

    if (id == 7)
        return 0;

    double inv_p = 4 * M_PI * light.rad * light.rad;
    double inv_d2 = 1.0 / d2;

    double cos_the = i.dot(n);
    double cos_phi = i.dot(w * -1.0);
    if (cos_the > 0 && cos_phi > 0) {
        return obj.brdf.eval(n, o, i).mult(light.e) *
               static_cast<float>(cos_the * cos_phi * inv_d2 * inv_p);
    } else {
        return spectrum();
    }
}

spectrum indirect(const vec &n, const vec &o, const vec &p_hit, const sphere &obj, int depth) {
    vec i;
    double pdf;
    obj.brdf.sample(n, o, i, pdf);
    if (std::abs(pdf) < 1e-4) {
        return spectrum();
    } else {
        spectrum distri = obj.brdf.eval(n, o, i);
        if (distri.black()) {
            return spectrum();
        }
        spectrum li = pt_received_radiance(ray(p_hit, i), depth + 1);
        double cos_the = i.dot(n);
        return distri.mult(li) * static_cast<float>(cos_the / pdf);
    }
}

spectrum radiance(const vec &n, const vec &o, const vec &p_hit, const sphere &obj, int id) {
    return obj.e + direct(n, o, p_hit, obj, id) + indirect(n, o, p_hit, obj, 2);
}

spectrum pt_received_radiance(const ray &r, int depth) {
    double t;   // Distance to intersection
    int id = 0; // id of intersected sphere

    if (!intersect(r, t, id))
        return spectrum();           // if miss, return black
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
    spectrum Ld = direct(n, o, x, obj, id);

    // Mutation.
    static const int mutate_depth = 3;
    double p_survive = 0.5;
    if (depth >= mutate_depth) {
        if (rng() >= p_survive)
            return Ld;
    } else
        p_survive = 1;

    // Indirect illuminant.
    spectrum Li = indirect(n, o, x, obj, depth);
    spectrum L = Ld + Li * static_cast<float>(1 / p_survive);
    return L;
}

/*
 * BDPT.
 */
struct path {
    struct vertex {
        vertex() = default;
        vertex(vertex const &) = default;
        vec hit_point;
        vec hit_point_normal;
        vec dir_to_prev;
        sphere const *obj = nullptr;
        double t = 0;
        spectrum light_transport;
        int id;
    };

    unsigned length() const { return static_cast<unsigned>(vertices.size()); }

    std::vector<vertex> vertices;
};

path sample_path(ray r, unsigned max_depth) {
    path p;
    spectrum light_transport(1.0, 1.0, 1.0);
    for (unsigned i = 0; i < max_depth; i++) {
        double t;
        int id;
        if (!intersect(r, t, id)) {
            break;
        }
        sphere const &obj = spheres[id];
        vec x = r.o + r.d * t;
        vec dir_to_prev = r.d * -1.0;
        vec n = (x - obj.p).normalize();
        if (n.dot(dir_to_prev) < 0)
            n = n * -1.0;

        path::vertex v;
        v.dir_to_prev = dir_to_prev;
        v.hit_point = x;
        v.hit_point_normal = n;
        v.obj = &obj;
        v.t = t;
        v.id = id;
        v.light_transport = light_transport;
        p.vertices.push_back(v);

        double path_dens;
        vec dir_to_next;
        obj.brdf.sample(n, dir_to_prev, dir_to_next, path_dens);
        if (std::abs(path_dens) < 1e-4) {
            break;
        }
        spectrum distri = obj.brdf.eval(n, dir_to_prev, dir_to_next);
        if (distri.black()) {
            break;
        }
        light_transport = light_transport.mult(distri * static_cast<float>(dir_to_next.dot(n))) *
                          static_cast<float>(1.0 / path_dens);
        r = ray(x, dir_to_next);
    }
    return p;
}

ray sample_light_ray(vec *normal, double *spatial_dens, double *dir_dens) {
    sphere const &light_ball = spheres[7];

    double u = rng();
    double v = rng();

    double pos_z = 2 * u - 1;
    double pos_x = std::sqrt(1 - pos_z * pos_z) * std::cos(2 * M_PI * v);
    double pos_y = std::sqrt(1 - pos_z * pos_z) * std::sin(2 * M_PI * v);
    *normal = vec(pos_x, pos_y, pos_z);

    double dir_z = std::sqrt(rng());
    double rad = std::sqrt(1.0 - dir_z * dir_z);
    double phi = 2.0 * M_PI * rng();
    double dir_x = rad * std::cos(phi);
    double dir_y = rad * std::sin(phi);

    vec b1, b2;
    create_local_coord(*normal, b1, b2);

    ray r(light_ball.p + *normal * light_ball.rad, b1 * dir_x + b2 * dir_y + *normal * dir_z);

    *spatial_dens = 1.0 / (4 * M_PI * light_ball.rad * light_ball.rad);
    *dir_dens = r.d.dot(*normal) / M_PI;
    return r;
}

spectrum bdpt_connect_and_estimate(path const &cam_path, path const &light_path,
                                   ray const &light_ray, vec const &light_normal,
                                   double light_spatial_dens, double light_dir_dens) {
    spectrum rad;
    for (unsigned path_len = 1; path_len <= cam_path.length() + light_path.length() + 1;
         path_len++) {
        unsigned cam_plen = std::min(path_len - 1, cam_path.length());
        unsigned light_plen = path_len - 1 - cam_plen;

        spectrum partition_rad_sum;
        float partition_weight_sum = 0;
        while (static_cast<int>(cam_plen) >= 0 && light_plen <= light_path.length()) {
            if (light_plen == 0 && cam_plen == 0) {
                // We have only the connection path, if it exists, which connects one vertex from
                // the camera and one vertex from the light.
                if (cam_path.vertices[cam_plen].id == 7) {
                    partition_rad_sum = partition_rad_sum + cam_path.vertices[cam_plen].obj->e;
                }
                partition_weight_sum += 1;
            } else if (light_plen == 0) {
                path::vertex cam_join_vert = cam_path.vertices[cam_plen - 1];

                vec i = light_ray.o - cam_join_vert.hit_point;
                double d2 = i.dot(i);
                double d = std::sqrt(d2);

                i = i * (1.0 / d);

                // Occlusion test.
                spectrum transported_importance;
                double t;
                int id2;
                intersect(ray(cam_join_vert.hit_point, i), t, id2);

                double inv_d2 = 1.0 / d2;
                double cos_hit = i.dot(cam_join_vert.hit_point_normal);
                double cos_light = (i * -1).dot(light_normal);
                if (t > d - 1e-5 && cos_light > 0 && cos_hit > 0) {
                    spectrum distri = cam_join_vert.obj->brdf.eval(cam_join_vert.hit_point_normal,
                                                                   cam_join_vert.dir_to_prev, i);
                    transported_importance = distri.mult(spheres[7].e) *
                                             static_cast<float>(cos_hit * cos_light * inv_d2 *
                                                                (1.0 / light_spatial_dens));
                }

                // compute light transportation for camera subpath.
                spectrum path_rad = transported_importance.mult(cam_join_vert.light_transport);
                partition_rad_sum = partition_rad_sum + path_rad;
                partition_weight_sum += 1;
            } else if (cam_plen == 0) {
                // The chance of the light path hitting the camera is zero.
                ;
            } else {
                path::vertex light_join_vert = light_path.vertices[light_plen - 1];
                path::vertex cam_join_vert = cam_path.vertices[cam_plen - 1];
                vec join_path = cam_join_vert.hit_point - light_join_vert.hit_point;
                double join_distance = std::sqrt(join_path.dot(join_path));
                join_path = join_path * (1.0 / join_distance);

                ray join_ray(light_join_vert.hit_point, join_path);
                double t;
                int id2;
                intersect(join_ray, t, id2);
                double cos_wo = light_join_vert.hit_point_normal.dot(join_path);
                double cos_wi = cam_join_vert.hit_point_normal.dot(join_path * -1);
                if (cos_wo > 0.0 && cos_wi > 0.0 && t > join_distance - 1e-5) {
                    // compute light transportation for light subpath.
                    spectrum light_emission =
                        spheres[7].e * static_cast<float>(light_normal.dot(light_ray.d) /
                                                          (light_dir_dens * light_spatial_dens));
                    spectrum light_subpath_importance =
                        light_emission.mult(light_join_vert.light_transport);

                    // transport light over the join path.
                    float to_area_differential =
                        static_cast<float>(cos_wi * cos_wo / (join_distance * join_distance));
                    spectrum light_join_weight = light_join_vert.obj->brdf.eval(
                        light_join_vert.hit_point_normal, join_path, light_join_vert.dir_to_prev);
                    spectrum cam_join_weight = cam_join_vert.obj->brdf.eval(
                        cam_join_vert.hit_point_normal, cam_join_vert.dir_to_prev, join_path * -1);
                    spectrum transported_importance =
                        light_subpath_importance.mult(light_join_weight).mult(cam_join_weight) *
                        to_area_differential;

                    // compute light transportation for camera subpath.
                    spectrum cam_subpath_radiance =
                        transported_importance.mult(cam_join_vert.light_transport);

                    partition_rad_sum = partition_rad_sum + cam_subpath_radiance;
                }
                partition_weight_sum += 1;
            }

            light_plen++;
            cam_plen--;
        }

        if (partition_weight_sum > 0) {
            rad = rad + partition_rad_sum * (1.0f / partition_weight_sum);
        }
    }
    return rad;
}

spectrum bdpt_received_radiance(const ray &r) {
    path cam_path = sample_path(r, /*max_depth=*/4);
    double light_spatial_dens, light_dir_dens;
    vec light_normal;
    ray light_ray = sample_light_ray(&light_normal, &light_spatial_dens, &light_dir_dens);
    path light_path = sample_path(light_ray, /*max_depth=*/4);
    return bdpt_connect_and_estimate(cam_path, light_path, light_ray, light_normal,
                                     light_spatial_dens, light_dir_dens);
}

double to_lum(const vec &c) { return c.x * 0.299 + c.y * 0.587 + c.z * 0.114; }

double exposure(const std::vector<vec> &c) {
    double s = 0;
    for (unsigned i = 0; i < c.size(); i++) {
        s += std::log(to_lum(c[i]) + 1e-3);
    }
    s /= c.size();
    return std::exp(s);
}

void aces_tonemap(std::vector<vec> &c, double exposure) {
    const double A = 2.51;
    const double B = 0.03;
    const double C = 2.43;
    const double D = 0.59;
    const double E = 0.14;
    for (unsigned i = 0; i < c.size(); i++) {
        vec color = c[i] * exposure;
        c[i] = color.mult(color * A + vec(B, B, B))
                   .div(color.mult(color * C + vec(D, D, D)) + vec(E, E, E));
    }
}

/*
 * Main function (do not modify)
 */
#define NDEBUG 1

int main(int argc, char *argv[]) {
    int nworkers = omp_get_num_procs();
    omp_set_num_threads(nworkers);
    rng.init(static_cast<size_t>(nworkers));

    unsigned w = 1024, h = 768;
    unsigned samps = argc == 2 ? static_cast<unsigned>(atoi(argv[1]) / 4) : 128; // # samples
    vec cx = vec(w * .5135 / h), cy = (cx.cross(cam.d)).normalize() * .5135;
    std::vector<spectrum> c(w * h);

#if NDEBUG
#pragma omp parallel for schedule(dynamic, 1)
#endif
    for (unsigned y = 0; y < h; y++) {
        for (unsigned x = 0; x < w; x++) {
            const unsigned i = (h - y - 1) * w + x;

            for (unsigned sy = 0; sy < 2; ++sy) {
                for (unsigned sx = 0; sx < 2; ++sx) {
                    spectrum r;
                    for (unsigned s = 0; s < samps; s++) {
                        vec d = cx * (((sx + .5) / 2 + x) / w - .5) +
                                cy * (((sy + .5) / 2 + y) / h - .5) + cam.d;
                        r = r + pt_received_radiance(ray(cam.o, d.normalize()), 1) * (1.f / samps);
                        // r = r + bdpt_received_radiance(ray(cam.o, d.normalize())) * (1.f /
                        // samps);
                    }
                    c[i] = c[i] + spectrum(r.r, r.g, r.b) * .25;
                }
            }
        }
#if NDEBUG
#pragma omp critical
#endif
        fprintf(stderr, "\rRendering (%d spp) %6.2f%%", samps * 4, 100. * y / (h - 1));
    }
    fprintf(stderr, "\n");

    // aces_tonemap(c, exposure(c));

    // Write resulting image to a PPM file
    FILE *f = fopen("image.ppm", "w");
    fprintf(f, "P3\n%d %d\n%d\n", w, h, 255);
    for (unsigned i = 0; i < w * h; i++) {
        int x = to_int(c[i].r);
        int y = to_int(c[i].g);
        int z = to_int(c[i].b);
        fprintf(f, "%d %d %d ", x, y, z);
    }
    fclose(f);

    return 0;
}
