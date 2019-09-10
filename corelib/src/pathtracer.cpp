#include "pathtracer.h"
#include <iostream>

namespace {
/**
 * @brief The sampled_pathlet struct
 * Element of the smallest parition of a path.
 */
struct sampled_pathlet {
    // The path vector.
    e8util::vec3 o;

    // The density which this pathlet is being selected, conditioned on all
    // previous pathlets.
    float dens;

    // The vertex to anchor the vector in space.
    // Note that, the end of the vector is anchored rather than the beginning.
    e8::intersect_info vert;

    sampled_pathlet() {}

    sampled_pathlet(e8util::vec3 o, e8::intersect_info vert, float dens)
        : o(o), dens(dens), vert(vert) {}
};

/**
 * @brief The pathlet_transporter class
 * Incrementally transport light pathlet by pathlet.
 * @param FORWARD The direction in respect to the path where light is transported.
 */
template <bool FORWARD> class pathlet_transporter {
  public:
    /**
     * @brief pahtlet_transporter
     * @param path The path sample on which light is transported.
     * @param len The length of path.
     * @param src_rad Source radiance to transport.
     */
    pathlet_transporter(sampled_pathlet const *path, unsigned len, e8util::vec3 const &src_rad);

    e8util::vec3 next(e8util::vec3 const &appending_ray, float appending_ray_dens);
    e8util::vec3 next();

  private:
    e8util::vec3 const &src_rad;
    unsigned m_len;
    sampled_pathlet const *m_path;
};

/**
 * @brief sample_path Recursively sample then concatenate pathlets to form a path sample.
 */
unsigned sample_path(e8util::rng &rng, sampled_pathlet *sampled_path,
                     e8::if_path_space const &path_space, unsigned depth, unsigned max_depth) {
    if (depth == max_depth)
        return depth;
    float w_dens;
    e8util::vec3 const &i = sampled_path[depth - 1].vert.mat->sample(
        rng, sampled_path[depth - 1].vert.normal, -sampled_path[depth - 1].o, w_dens);
    e8::intersect_info const &next_vert =
        path_space.intersect(e8util::ray(sampled_path[depth - 1].vert.vertex, i));
    if (next_vert.valid) {
        sampled_path[depth] = sampled_pathlet(i, next_vert, w_dens);
        return sample_path(rng, sampled_path, path_space, depth + 1, max_depth);
    } else {
        return depth;
    }
}

/**
 * @brief sample_path Sample a path X conditioned on X0 = r0 and max_depth.
 * @param rng Random number generator.
 * @param sampled_path Result, path sample.
 * @param r0 The bootstrap path to condition on.
 * @param dens0 The density of the pathlet r0.
 * @param path_space The path space to sample from.
 * @param max_depth Maximum path length condition.
 * @return Actual path length of the sampled_path. It may not be max_depth in the case when the
 * light escapes out of the path_space during sampling.
 */
unsigned sample_path(e8util::rng &rng, sampled_pathlet *sampled_path, e8util::ray const &r0,
                     float dens0, e8::if_path_space const &path_space, unsigned max_depth) {
    e8::intersect_info const &vert0 = path_space.intersect(r0);
    if (!vert0.valid) {
        return 0;
    } else {
        sampled_path[0] = sampled_pathlet(r0.v(), vert0, dens0);
        return sample_path(rng, sampled_path, path_space, 1, max_depth);
    }
}

/**
 * @brief transport_subpath Compute a light transport sample over the sampled_path[:sub_path_len].
 * @param src_rad Source radiance to transport.
 * @param appending_ray Append a pathlet vector to the ending of the
 * sampled_path.
 * @param appending_ray_dens The density of the appended pathlet.
 * @param sampled_path The path sample that the sub-path is to be taken from.
 * @param sub_path_len The length of the prefix subpath taken from sampled_path.
 * @param forward The direction in which light is transported.
 * @return The amount of radiance transported.
 */
e8util::vec3 transport_subpath(e8util::vec3 const &src_rad, e8util::vec3 const &appending_ray,
                               float appending_ray_dens, sampled_pathlet const *sampled_path,
                               unsigned sub_path_len, bool forward) {
    if (sub_path_len == 0)
        return src_rad;
    if (forward) {
        e8util::vec3 transport = src_rad;
        for (unsigned k = 0; k < sub_path_len - 1; k++) {
            transport *= sampled_path[k].vert.mat->eval(sampled_path[k].vert.normal,
                                                        sampled_path[k + 1].o, -sampled_path[k].o) *
                         sampled_path[k].vert.normal.inner(-sampled_path[k].o) /
                         sampled_path[k + 1].dens;
        }
        return transport *
               sampled_path[sub_path_len - 1].vert.mat->eval(
                   sampled_path[sub_path_len - 1].vert.normal, appending_ray,
                   -sampled_path[sub_path_len - 1].o) *
               sampled_path[sub_path_len - 1].vert.normal.inner(-sampled_path[sub_path_len - 1].o) /
               appending_ray_dens;
    } else {
        e8util::vec3 transport = src_rad *
                                 sampled_path[sub_path_len - 1].vert.mat->eval(
                                     sampled_path[sub_path_len - 1].vert.normal,
                                     -sampled_path[sub_path_len - 1].o, appending_ray) *
                                 sampled_path[sub_path_len - 1].vert.normal.inner(appending_ray) /
                                 appending_ray_dens;
        for (int k = static_cast<int>(sub_path_len) - 2; k >= 0; k--) {
            transport *= sampled_path[k].vert.mat->eval(sampled_path[k].vert.normal,
                                                        -sampled_path[k].o, sampled_path[k + 1].o) *
                         sampled_path[k].vert.normal.inner(sampled_path[k + 1].o) /
                         sampled_path[k + 1].dens;
        }
        return transport;
    }
}

/**
 * @brief subpath_density The path density of sampled_path[path_start:path_end]
 * @param src_dens The density of the initial vertex.
 * @param sampled_path The path sample that the sub-path is to be taken from.
 * @param path_start
 * @param path_end
 * @return
 */
float subpath_density(float src_dens, sampled_pathlet const *sampled_path, unsigned path_start,
                      unsigned path_end) {
    if (path_end == 0)
        return 0.0f;
    float dens = src_dens;
    for (unsigned k = path_start; k < path_end; k++) {
        dens *= sampled_path[k].dens * sampled_path[k].vert.normal.inner(-sampled_path[k].o) /
                (sampled_path[k].vert.t * sampled_path[k].vert.t);
    }
    return dens;
}

} // namespace

e8::if_pathtracer::if_pathtracer() {}

e8::if_pathtracer::~if_pathtracer() {}

e8::position_pathtracer::position_pathtracer() {}

e8::position_pathtracer::~position_pathtracer() {}

std::vector<e8util::vec3> e8::position_pathtracer::sample(
    e8util::rng &, std::vector<e8util::ray> const &rays, if_path_space const &path_space,
    if_light_sources const & /* light_sources */, unsigned /* n */) const {
    e8util::aabb const &aabb = path_space.aabb();
    e8util::vec3 const &range = aabb.max() - aabb.min();
    std::vector<e8util::vec3> rad(rays.size());
    for (unsigned i = 0; i < rays.size(); i++) {
        e8util::ray const &ray = rays[i];
        e8::intersect_info const &vert = path_space.intersect(ray);
        if (vert.valid) {
            e8util::vec3 const &p = (vert.vertex - aabb.min()) / range;
            rad[i] = e8util::vec3({p(0), p(1), p(2)});
        } else
            rad[i] = 0.0f;
    }
    return rad;
}

e8::normal_pathtracer::normal_pathtracer() {}

e8::normal_pathtracer::~normal_pathtracer() {}

std::vector<e8util::vec3>
e8::normal_pathtracer::sample(e8util::rng &, std::vector<e8util::ray> const &rays,
                              if_path_space const &path_space,
                              if_light_sources const & /* light_sources */, unsigned) const {
    std::vector<e8util::vec3> rad(rays.size());
    for (unsigned i = 0; i < rays.size(); i++) {
        e8util::ray const &ray = rays[i];
        e8::intersect_info const &vert = path_space.intersect(ray);
        if (vert.valid) {
            e8util::vec3 const &p = (vert.normal + 1.0f) / 2.0f;
            rad[i] = e8util::vec3({p(0), p(1), p(2)});
        } else
            rad[i] = 0.0f;
    }
    return rad;
}

e8::direct_pathtracer::direct_pathtracer() {}

e8::direct_pathtracer::~direct_pathtracer() {}

e8::if_light const *
e8::direct_pathtracer::sample_illum_source(e8util::rng &rng, e8util::vec3 &p, e8util::vec3 &n,
                                           float &density, e8::intersect_info const &,
                                           if_light_sources const &light_sources) const {
    // sample light.
    float light_pdf;
    if_light const *light = light_sources.sample_light(rng, light_pdf);

    float source_pdf;
    light->sample(rng, source_pdf, p, n);

    density = source_pdf * light_pdf;
    return light;
}

e8util::vec3 e8::direct_pathtracer::transport_illum_source(if_light const &light,
                                                           e8util::vec3 const &p_illum,
                                                           e8util::vec3 const &n_illum,
                                                           e8::intersect_info const &target_vert,
                                                           e8util::vec3 const &target_o_ray,
                                                           if_path_space const &path_space) const {
    // construct light path.
    e8util::vec3 const &l = target_vert.vertex - p_illum;
    e8util::vec3 const &illum = light.eval(l, n_illum);
    if (illum == 0.0f)
        return 0.0f;

    float distance = l.norm();
    e8util::vec3 const &i = -l / distance;

    // evaluate.
    float cos_w = i.inner(target_vert.normal);
    e8util::ray light_ray(target_vert.vertex, i);
    float t;
    if (!path_space.has_intersect(light_ray, 1e-4f, distance - 1e-3f, t)) {
        return illum * target_vert.mat->eval(target_vert.normal, target_o_ray, i) * cos_w;
    } else {
        return 0.0f;
    }
}

e8util::vec3 e8::direct_pathtracer::sample_direct_illum(e8util::rng &rng,
                                                        e8util::vec3 const &target_o_ray,
                                                        e8::intersect_info const &target_vert,
                                                        if_path_space const &path_space,
                                                        if_light_sources const &light_sources,
                                                        unsigned multi_light_samps) const {
    e8util::vec3 rad;
    for (unsigned k = 0; k < multi_light_samps; k++) {
        e8util::vec3 p, n;
        float density;
        e8::if_light const *light =
            sample_illum_source(rng, p, n, density, target_vert, light_sources);
        rad +=
            transport_illum_source(*light, p, n, target_vert, target_o_ray, path_space) / density;
    }
    return rad / multi_light_samps;
}

std::vector<e8util::vec3> e8::direct_pathtracer::sample(e8util::rng &rng,
                                                        std::vector<e8util::ray> const &rays,
                                                        if_path_space const &path_space,
                                                        if_light_sources const &light_sources,
                                                        unsigned multi_light_samps) const {
    std::vector<e8util::vec3> rad(rays.size());
    for (unsigned i = 0; i < rays.size(); i++) {
        e8util::ray const &ray = rays[i];
        e8::intersect_info const &vert = path_space.intersect(ray);
        if (vert.valid) {
            // compute radiance.
            rad[i] = sample_direct_illum(rng, -ray.v(), vert, path_space, light_sources,
                                         multi_light_samps);
            if (vert.light)
                rad[i] += vert.light->emission(-ray.v(), vert.normal);
        }
    }
    return rad;
}

e8::unidirect_pathtracer::unidirect_pathtracer() {}

e8::unidirect_pathtracer::~unidirect_pathtracer() {}

e8util::vec3 e8::unidirect_pathtracer::sample_indirect_illum(
    e8util::rng &rng, e8util::vec3 const &o, e8::intersect_info const &vert,
    if_path_space const &path_space, if_light_sources const &light_sources, unsigned depth,
    unsigned multi_light_samps, unsigned multi_indirect_samps) const {
    static const int mutate_depth = 2;
    float p_survive = 0.5f;
    if (depth >= mutate_depth) {
        if (rng.draw() >= p_survive)
            return 0.0f;
    } else
        p_survive = 1;

    if (depth >= 1)
        multi_indirect_samps = 1;

    // direct.
    e8util::vec3 const &direct =
        sample_direct_illum(rng, o, vert, path_space, light_sources, multi_light_samps);

    // indirect.
    float mat_pdf;
    e8util::vec3 multi_indirect;
    for (unsigned k = 0; k < multi_indirect_samps; k++) {
        e8util::vec3 const &i = vert.mat->sample(rng, vert.normal, o, mat_pdf);
        e8::intersect_info const &indirect_vert = path_space.intersect(e8util::ray(vert.vertex, i));
        if (indirect_vert.valid) {
            e8util::vec3 const &indirect =
                sample_indirect_illum(rng, -i, indirect_vert, path_space, light_sources, depth + 1,
                                      multi_light_samps, multi_indirect_samps);
            e8util::vec3 const &brdf = vert.mat->eval(vert.normal, o, i);
            float cos_w = vert.normal.inner(i);
            multi_indirect += indirect * brdf * cos_w / mat_pdf;
        }
    }

    return (direct + multi_indirect / multi_indirect_samps) / p_survive;
}

std::vector<e8util::vec3> e8::unidirect_pathtracer::sample(e8util::rng &rng,
                                                           std::vector<e8util::ray> const &rays,
                                                           if_path_space const &path_space,
                                                           if_light_sources const &light_sources,
                                                           unsigned /* n */) const {
    std::vector<e8util::vec3> rad(rays.size());
    for (unsigned i = 0; i < rays.size(); i++) {
        e8util::ray const &ray = rays[i];
        e8::intersect_info const &vert = path_space.intersect(ray);
        if (vert.valid) {
            // compute radiance.
            e8util::vec3 const &p2_inf =
                sample_indirect_illum(rng, -ray.v(), vert, path_space, light_sources, 0, 1, 1);
            if (vert.light)
                rad[i] = p2_inf + vert.light->emission(-ray.v(), vert.normal);
            else
                rad[i] = p2_inf;
        }
    }
    return rad;
}

e8::bidirect_lt2_pathtracer::bidirect_lt2_pathtracer() {}

e8::bidirect_lt2_pathtracer::~bidirect_lt2_pathtracer() {}

e8util::vec3 e8::bidirect_lt2_pathtracer::join_with_light_paths(
    e8util::rng &rng, e8util::vec3 const &o, e8::intersect_info const &poi,
    if_path_space const &path_space, if_light_sources const &light_sources,
    unsigned cam_path_len) const {
    e8util::vec3 const &p1_direct = sample_direct_illum(rng, o, poi, path_space, light_sources, 1);

    // sample light.
    float light_pdf, source_p_pdf, source_w_pdf;
    if_light const *light = light_sources.sample_light(rng, light_pdf);
    e8util::vec3 p, n, w;
    light->sample(rng, source_p_pdf, source_w_pdf, p, n, w);
    e8util::ray light_path(p, w);
    e8::intersect_info const &light_info = path_space.intersect(light_path);
    if (!light_info.valid)
        return 0.0f;

    // construct light path.
    e8util::vec3 const &illum = light->emission(w, n) / (light_pdf * source_p_pdf * source_w_pdf);

    e8::intersect_info terminate = light_info;
    e8util::vec3 tray = -w;
    e8util::vec3 const &light_illum = illum;

    // evaluate the area integral.
    e8util::vec3 join_path = poi.vertex - terminate.vertex;
    float distance = join_path.norm();
    join_path = join_path / distance;
    e8util::ray join_ray(terminate.vertex, join_path);
    float cos_w2 = terminate.normal.inner(tray);
    float cos_wo = terminate.normal.inner(join_path);
    float cos_wi = poi.normal.inner(-join_path);
    float t;
    e8util::vec3 p2_direct;
    if (cos_wo > 0.0f && cos_wi > 0.0f && cos_w2 > 0.0f &&
        !path_space.has_intersect(join_ray, 1e-4f, distance - 1e-3f, t)) {
        e8util::vec3 f2 =
            light_illum * terminate.mat->eval(terminate.normal, join_path, tray) * cos_w2;
        p2_direct =
            f2 * cos_wo / (distance * distance) * poi.mat->eval(poi.normal, o, -join_path) * cos_wi;
        if (cam_path_len == 0)
            return p1_direct + 0.5f * p2_direct;
        else
            return 0.5f * (p1_direct + p2_direct);
    }
    return p1_direct;
}

e8util::vec3 e8::bidirect_lt2_pathtracer::sample_indirect_illum(
    e8util::rng &rng, e8util::vec3 const &o, e8::intersect_info const &vert,
    if_path_space const &path_space, if_light_sources const &light_sources, unsigned depth) const {
    static const unsigned mutate_depth = 1;
    float p_survive = 0.5f;
    if (depth >= mutate_depth) {
        if (rng.draw() >= p_survive)
            return 0.0f;
    } else
        p_survive = 1;

    e8util::vec3 const &bidirect =
        join_with_light_paths(rng, o, vert, path_space, light_sources, depth);

    // indirect.
    float mat_pdf;
    e8util::vec3 const &i = vert.mat->sample(rng, vert.normal, o, mat_pdf);
    e8::intersect_info const &indirect_info = path_space.intersect(e8util::ray(vert.vertex, i));
    e8util::vec3 r;
    if (indirect_info.valid) {
        e8util::vec3 const &indirect =
            sample_indirect_illum(rng, -i, indirect_info, path_space, light_sources, depth + 1);
        e8util::vec3 const &brdf = vert.mat->eval(vert.normal, o, i);
        float cos_w = vert.normal.inner(i);
        if (cos_w < 0.0f)
            return 0.0f;
        r = indirect * brdf * cos_w / mat_pdf;
    }
    return (bidirect + r) / p_survive;
}

std::vector<e8util::vec3> e8::bidirect_lt2_pathtracer::sample(e8util::rng &rng,
                                                              std::vector<e8util::ray> const &rays,
                                                              if_path_space const &path_space,
                                                              if_light_sources const &light_sources,
                                                              unsigned) const {
    std::vector<e8util::vec3> rad(rays.size());
    for (unsigned i = 0; i < rays.size(); i++) {
        e8util::ray const &ray = rays[i];
        e8::intersect_info const &vert = path_space.intersect(ray);
        if (vert.valid) {
            // compute radiance.
            e8util::vec3 const &p2_inf =
                sample_indirect_illum(rng, -ray.v(), vert, path_space, light_sources, 0);
            if (vert.light)
                rad[i] = p2_inf + vert.light->emission(-ray.v(), vert.normal);
            else
                rad[i] = p2_inf;
        }
    }
    return rad;
}

e8::bidirect_mis_pathtracer::bidirect_mis_pathtracer() {}

e8::bidirect_mis_pathtracer::~bidirect_mis_pathtracer() {}

e8::if_light const *
e8::bidirect_mis_pathtracer::sample_illum_source(e8util::rng &rng, e8util::vec3 &p, e8util::vec3 &n,
                                                 e8util::vec3 &w, float &density, float &w_density,
                                                 if_light_sources const &light_sources) const {
    // sample light.
    float light_dens;
    if_light const *light = light_sources.sample_light(rng, light_dens);

    float source_dens;
    light->sample(rng, source_dens, w_density, p, n, w);

    density = source_dens * light_dens;
    return light;
}

e8util::vec3 e8::bidirect_mis_pathtracer::sample_all_subpaths(
    sampled_pathlet const *cam_path, unsigned cam_path_len, sampled_pathlet const *light_path,
    unsigned light_path_len, e8util::vec3 const &light_p, e8util::vec3 const &light_n,
    float pdf_light_p_dens, if_light const &light, if_path_space const &path_space) const {
    float weights[m_max_path_len * 2 + 2] = {0};
    e8util::vec3 subpath_rads[m_max_path_len * 2 + 2];

    // sweep all path lengths and strategies by pairing camera and light subpaths.
    // no camera vertex generation, yet. both i, j are one-offset path lengths.
    for (unsigned i = 1; i <= cam_path_len; i++) {
        for (unsigned j = 0; j <= light_path_len; j++) {
            e8util::vec3 path_rad;
            float weight;
            if (j == 0) {
                // direct light sampling.
                float dens = pdf_light_p_dens; // direction was not chosen by random process.
                e8util::vec3 transported_light_illum =
                    transport_illum_source(light, light_p, light_n, cam_path[i - 1].vert,
                                           -cam_path[i - 1].o, path_space) /
                    dens;

                // compute light transportation for camera subpath.
                path_rad = transport_subpath(transported_light_illum, cam_path[i - 1].o,
                                             cam_path[i - 1].dens, cam_path, i - 1, false) /
                           cam_path[0].dens;

                weight = subpath_density(1.0f, cam_path, 1, i) * dens;
            } else {
                e8util::vec3 join_path =
                    cam_path[i - 1].vert.vertex - light_path[j - 1].vert.vertex;
                float distance = join_path.norm();
                join_path = join_path / distance;
                e8util::ray join_ray(cam_path[i - 1].vert.vertex, join_path);
                float cos_w2 = light_path[j].vert.normal.inner(-light_path[j - 1].o);
                float cos_wo = light_path[j].vert.normal.inner(join_path);
                float cos_wi = cam_path[i - 1].vert.normal.inner(-join_path);
                float t;
                if (cos_wo > 0.0f && cos_wi > 0.0f && cos_w2 > 0.0f &&
                    !path_space.has_intersect(join_ray, 1e-4f, distance - 1e-3f, t)) {
                    // compute light transportation for light subpath.
                    e8util::vec3 light_illum = light.emission(light_path[0].o, light_n) /
                                               (light_path[0].dens * pdf_light_p_dens);
                    e8util::vec3 light_subpath_rad =
                        transport_subpath(light_illum, join_path, 1.0f, light_path, j, true);

                    // transport light for the join path.
                    e8util::vec3 transported_light_illum =
                        light_subpath_rad * cos_wo / (distance * distance);

                    // compute light transportation for camera subpath.
                    path_rad = transport_subpath(transported_light_illum, -join_path, 1.0f,
                                                 cam_path, i, false) /
                               cam_path[0].dens;
                }

                float cam_weight = subpath_density(1.0f, cam_path, 1, i);
                float light_weight = subpath_density(pdf_light_p_dens, light_path, 0, j);
                weight = cam_weight * light_weight;
            }

            weights[i + j + 1] += weight;
            subpath_rads[i + j + 1] += path_rad * weight;
        }
    }

    e8util::vec3 rad;
    for (unsigned i = 1; i <= cam_path_len; i++) {
        for (unsigned j = 0; j <= light_path_len; j++) {
            if (weights[i + j + 1] > 0)
                rad += subpath_rads[i + j + 1] / weights[i + j + 1];
        }
    }
    return rad;
}

e8util::vec3 e8::bidirect_mis_pathtracer::sample_all_subpaths_opti(
    sampled_pathlet const *cam_path, unsigned max_cam_path_len, sampled_pathlet const *light_path,
    unsigned max_light_path_len, e8util::vec3 const &light_p, e8util::vec3 const &light_n,
    float pdf_light_p_dens, if_light const &light, if_path_space const &path_space) const {
    if (max_cam_path_len == 0) {
        // Nothing to sample;
        return 0.0f;
    }

    e8util::vec3 rad;

    // sweep all path lengths and strategies by pairing camera and light subpaths.
    // no camera vertex generation, yet. both cam_plen, light_plen are one-offset path lengths.
    for (unsigned plen = 1; plen <= max_cam_path_len + max_light_path_len + 1; plen++) {
        unsigned cam_plen = std::min(plen - 1, max_cam_path_len);
        unsigned light_plen = plen - 1 - cam_plen;

        unsigned num_valid_samps = 0;
        e8util::vec3 paritition_rad_sum;
        while (light_plen < plen && light_plen <= max_light_path_len) {
            e8util::vec3 path_rad;

            if (light_plen == 0 && cam_plen == 0) {
                // We have only the connection path, if it exists, which connects
                // one vertex from the camera and one vertex from the light.
                if (cam_path[0].vert.light != nullptr) {
                    path_rad =
                        cam_path[0].vert.light->emission(-cam_path[0].o, cam_path[0].vert.normal);
                    num_valid_samps++;
                }
            } else if (light_plen == 0) {
                float dens = pdf_light_p_dens; // direction was not chosen by random process.
                e8util::vec3 transported_light_illum =
                    transport_illum_source(light, light_p, light_n, cam_path[cam_plen - 1].vert,
                                           -cam_path[cam_plen - 1].o, path_space) /
                    dens;

                // compute light transportation for camera subpath.
                path_rad =
                    transport_subpath(transported_light_illum, cam_path[cam_plen - 1].o,
                                      cam_path[cam_plen - 1].dens, cam_path, cam_plen - 1, false) /
                    cam_path[0].dens;
                num_valid_samps++;
            } else if (cam_plen == 0) {
                path_rad = 0.0f;
            } else {
                e8util::vec3 join_path =
                    cam_path[cam_plen - 1].vert.vertex - light_path[light_plen - 1].vert.vertex;
                float distance = join_path.norm();
                join_path = join_path / distance;
                e8util::ray join_ray(cam_path[cam_plen - 1].vert.vertex, join_path);
                float cos_w2 =
                    light_path[light_plen].vert.normal.inner(-light_path[light_plen - 1].o);
                float cos_wo = light_path[light_plen].vert.normal.inner(join_path);
                float cos_wi = cam_path[light_plen - 1].vert.normal.inner(-join_path);
                float t;
                if (cos_wo > 0.0f && cos_wi > 0.0f && cos_w2 > 0.0f &&
                    !path_space.has_intersect(join_ray, 1e-4f, distance - 1e-3f, t)) {
                    // compute light transportation for light subpath.
                    e8util::vec3 light_illum = light.emission(light_path[0].o, light_n) /
                                               (light_path[0].dens * pdf_light_p_dens);
                    e8util::vec3 light_subpath_rad = transport_subpath(
                        light_illum, join_path, 1.0f, light_path, light_plen, true);

                    // transport light for the join path.
                    e8util::vec3 transported_light_illum =
                        light_subpath_rad * cos_wo / (distance * distance);

                    // compute light transportation for camera subpath.
                    path_rad = transport_subpath(transported_light_illum, -join_path, 1.0f,
                                                 cam_path, cam_plen, false) /
                               cam_path[0].dens;
                    num_valid_samps++;
                }
            }

            paritition_rad_sum += path_rad;
            light_plen++;
            cam_plen--;
        }

        if (num_valid_samps > 0)
            rad += paritition_rad_sum / num_valid_samps;
    }
    return rad;
}

std::vector<e8util::vec3> e8::bidirect_mis_pathtracer::sample(e8util::rng &rng,
                                                              std::vector<e8util::ray> const &rays,
                                                              if_path_space const &path_space,
                                                              if_light_sources const &light_sources,
                                                              unsigned) const {
    std::vector<e8util::vec3> rad(rays.size());
    for (unsigned i = 0; i < rays.size(); i++) {
        // initialize the first paths for both camera and light.
        e8util::ray const &cam_path0 = rays[i];
        e8util::vec3 light_p;
        e8util::vec3 light_n;
        e8util::vec3 light_w;
        float light_dens;
        float light_w_dens;
        if_light const *light = sample_illum_source(rng, light_p, light_n, light_w, light_dens,
                                                    light_w_dens, light_sources);
        e8util::ray const &light_path0 = e8util::ray(light_p, light_w);

        // produce both camera and light paths.
        sampled_pathlet cam_path[m_max_path_len];
        unsigned cam_path_len =
            sample_path(rng, cam_path, cam_path0, 1.0f, path_space, m_max_path_len);

        sampled_pathlet light_path[m_max_path_len];
        unsigned light_path_len =
            sample_path(rng, light_path, light_path0, light_w_dens, path_space, m_max_path_len);

        // compute radiance for different strategies.
        rad[i] = sample_all_subpaths_opti(cam_path, cam_path_len, light_path, light_path_len,
                                          light_p, light_n, light_dens, *light, path_space);
    }
    return rad;
}
