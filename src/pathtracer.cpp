#include <iostream>
#include "pathtracer.h"

e8::if_pathtracer::if_pathtracer()
{

}

e8::if_pathtracer::~if_pathtracer()
{
}



e8::position_pathtracer::position_pathtracer()
{
}

e8::position_pathtracer::~position_pathtracer()
{
}

std::vector<e8util::vec3>
e8::position_pathtracer::sample(e8util::rng&, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned) const
{
        e8util::aabb const& aabb = scene->aabb();
        e8util::vec3 const& range = aabb.max() - aabb.min();
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                e8util::ray const& ray = rays[i];
                e8::intersect_info const& info = scene->intersect(ray);
                if (info.valid) {
                        e8util::vec3 const& p = (info.vertex - aabb.min())/range;
                        rad[i] = e8util::vec3({p(0), p(1), p(2)});
                } else
                        rad[i] = 0.0f;
        }
        return rad;
}


e8::normal_pathtracer::normal_pathtracer()
{
}

e8::normal_pathtracer::~normal_pathtracer()
{
}

std::vector<e8util::vec3>
e8::normal_pathtracer::sample(e8util::rng&, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned) const
{
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                e8util::ray const& ray = rays[i];
                e8::intersect_info const& info = scene->intersect(ray);
                if (info.valid) {
                        e8util::vec3 const& p = (info.normal + 1.0f)/2.0f;
                        rad[i] = e8util::vec3({p(0), p(1), p(2)});
                } else
                        rad[i] = 0.0f;
        }
        return rad;
}

e8::direct_pathtracer::direct_pathtracer()
{
}

e8::direct_pathtracer::~direct_pathtracer()
{
}

e8util::vec3
e8::direct_pathtracer::sample_direct_illum(e8util::rng& rng,
                                           e8util::vec3 const& o,
                                           e8::intersect_info const& info,
                                           if_scene const* scene,
                                           unsigned multi_light_samps) const
{
        e8util::vec3 rad;
        for (unsigned k = 0; k < multi_light_samps; k ++) {
                // sample light.
                float light_pdf;
                if_light const* light = scene->sample_light(rng, light_pdf);

                float source_pdf;
                e8util::vec3 p, n;
                light->sample(rng, source_pdf, p, n);

                // construct light path.
                e8util::vec3 const& l = info.vertex - p;
                e8util::vec3 const& illum = light->eval(l, n)/(light_pdf*source_pdf);
                if (illum == 0.0f)
                        continue;

                float distance = l.norm();
                e8util::vec3 const& i = -l/distance;

                // evaluate.
                float cos_w = i.inner(info.normal);
                e8util::ray light_ray(info.vertex, i);
                float t;
                if (!scene->has_intersect(light_ray, 1e-4f, distance - 1e-3f, t))
                        rad += illum*info.mat->eval(info.normal, o, i)*cos_w;
        }
        return rad/multi_light_samps;
}

std::vector<e8util::vec3>
e8::direct_pathtracer::sample(e8util::rng& rng,
                              std::vector<e8util::ray> const& rays,
                              if_scene const* scene,
                              unsigned multi_light_samps) const
{
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                e8util::ray const& ray = rays[i];
                e8::intersect_info const& info = scene->intersect(ray);
                if (info.valid) {
                        // compute radiance.
                        rad[i] = sample_direct_illum(rng, -ray.v(), info, scene, multi_light_samps);
                        if (info.light)
                                rad[i] += info.light->emission(-ray.v(), info.normal);
                }
        }
        return rad;
}

e8::unidirect_pathtracer::unidirect_pathtracer()
{
}

e8::unidirect_pathtracer::~unidirect_pathtracer()
{
}

e8util::vec3
e8::unidirect_pathtracer::sample_indirect_illum(e8util::rng& rng,
                                                e8util::vec3 const& o,
                                                e8::intersect_info const& info,
                                                if_scene const* scene,
                                                unsigned depth,
                                                unsigned multi_light_samps,
                                                unsigned multi_indirect_samps) const
{
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
        e8util::vec3 const& direct = sample_direct_illum(rng, o, info, scene, multi_light_samps);

        // indirect.
        float mat_pdf;
        e8util::vec3 multi_indirect;
        for (unsigned k = 0; k < multi_indirect_samps; k ++) {
                e8util::vec3 const& i = info.mat->sample(rng, info.normal, o, mat_pdf);
                e8::intersect_info const& indirect_info = scene->intersect(e8util::ray(info.vertex, i));
                if (indirect_info.valid) {
                       e8util::vec3 const& indirect = sample_indirect_illum(rng, -i, indirect_info,
                                                                            scene,
                                                                            depth + 1,
                                                                            multi_light_samps,
                                                                            multi_indirect_samps);
                       e8util::vec3 const& brdf = info.mat->eval(info.normal, o, i);
                       float cos_w = info.normal.inner(i);
                       multi_indirect += indirect*brdf*cos_w/mat_pdf;
                }
        }

        return (direct + multi_indirect/multi_indirect_samps)/p_survive;
}

std::vector<e8util::vec3>
e8::unidirect_pathtracer::sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned) const
{
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                e8util::ray const& ray = rays[i];
                e8::intersect_info const& info = scene->intersect(ray);
                if (info.valid) {
                        // compute radiance.
                        e8util::vec3 const& p2_inf = sample_indirect_illum(rng, -ray.v(), info, scene, 0, 1, 1);
                        if (info.light)
                                rad[i] = p2_inf + info.light->emission(-ray.v(), info.normal);
                        else
                                rad[i] = p2_inf;
                }
        }
        return rad;
}

e8::bidirect_pathtracer::bidirect_pathtracer()
{
}

e8::bidirect_pathtracer::~bidirect_pathtracer()
{
}

e8util::vec3
e8::bidirect_pathtracer::join_with_light_paths(e8util::rng& rng,
                                               e8util::vec3 const& o,
                                               e8::intersect_info const& poi,
                                               if_scene const* scene,
                                               unsigned cam_path_len) const
{
        e8util::vec3 const& p1_direct = sample_direct_illum(rng, o, poi, scene, 1);

        // sample light.
        float light_pdf, source_pdf;
        if_light const* light = scene->sample_light(rng, light_pdf);
        e8util::vec3 p, n, w;
        light->sample(rng, source_pdf, p, n, w);
        e8util::ray light_path(p, w);
        e8::intersect_info const& light_info = scene->intersect(light_path);
        if (!light_info.valid)
                return 0.0f;

        // construct light path.
        e8util::vec3 const& l = light_info.vertex - p;
        e8util::vec3 const& illum = light->emission(w, n)/(light_pdf*source_pdf);

        e8::intersect_info terminate = light_info;
        e8util::vec3 tray = -w;
        e8util::vec3 const& light_illum = illum;

        // evaluate the area integral.
        e8util::vec3 join_path = poi.vertex - terminate.vertex;
        float distance = join_path.norm();
        join_path = join_path/distance;
        e8util::ray join_ray(terminate.vertex, join_path);
        float cos_w2 = terminate.normal.inner(tray);
        float cos_wo = terminate.normal.inner(join_path);
        float cos_wi = poi.normal.inner(-join_path);
        float t;
        e8util::vec3 p2_direct;
        if (cos_wo >= 0.0f &&
                        cos_wi >= 0.0f &&
                        cos_w2 >= 0.0f &&
                        !scene->has_intersect(join_ray, 1e-4f, distance - 1e-3f, t)) {
                e8util::vec3 f2 = light_illum*terminate.mat->eval(terminate.normal, join_path, tray)*cos_w2;
                p2_direct = f2*cos_wo/(distance*distance)*poi.mat->eval(poi.normal, o, -join_path)*cos_wi;
                if (cam_path_len == 0)
                        return p1_direct + 0.5f*p2_direct;
                else
                        return 0.5f*(p1_direct + p2_direct);
        }
        return p1_direct;
}

e8util::vec3
e8::bidirect_pathtracer::sample_indirect_illum(e8util::rng& rng,
                                                e8util::vec3 const& o,
                                                e8::intersect_info const& info,
                                                if_scene const* scene,
                                                unsigned depth) const
{
        static const unsigned mutate_depth = 1;
        float p_survive = 0.5f;
        if (depth >= mutate_depth) {
                if (rng.draw() >= p_survive)
                        return 0.0f;
        } else
                p_survive = 1;

        e8util::vec3 const& bidirect = join_with_light_paths(rng, o, info, scene, depth);

        // indirect.
        float mat_pdf;
        e8util::vec3 const& i = info.mat->sample(rng, info.normal, o, mat_pdf);
        e8::intersect_info const& indirect_info = scene->intersect(e8util::ray(info.vertex, i));
        e8util::vec3 r;
        if (indirect_info.valid) {
                e8util::vec3 const& indirect = sample_indirect_illum(rng, -i, indirect_info, scene, depth + 1);
                e8util::vec3 const& brdf = info.mat->eval(info.normal, o, i);
                float cos_w = info.normal.inner(i);
                if (cos_w < 0.0f)
                        return 0.0f;
                r = indirect*brdf*cos_w/mat_pdf;
        }
        return (bidirect + r)/p_survive;
}

std::vector<e8util::vec3>
e8::bidirect_pathtracer::sample(e8util::rng& rng,
                                std::vector<e8util::ray> const& rays,
                                if_scene const* scene,
                                unsigned) const
{
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                e8util::ray const& ray = rays[i];
                e8::intersect_info const& info = scene->intersect(ray);
                if (info.valid) {
                        // compute radiance.
                        e8util::vec3 const& p2_inf = sample_indirect_illum(rng, -ray.v(), info, scene, 0);
                        if (info.light)
                                rad[i] = p2_inf + info.light->emission(-ray.v(), info.normal);
                        else
                                rad[i] = p2_inf;
                }
        }
        return rad;
}


e8::bidirect_mis_pathtracer::bidirect_mis_pathtracer()
{
}

e8::bidirect_mis_pathtracer::~bidirect_mis_pathtracer()
{
}

e8util::vec3
e8::bidirect_mis_pathtracer::join_with_light_paths(e8util::rng& rng,
                                               e8util::vec3 const& o,
                                               e8::intersect_info const& poi,
                                               if_scene const* scene) const
{
        return 0.0f;
}

e8util::vec3
e8::bidirect_mis_pathtracer::sample_indirect_illum(e8util::rng& rng,
                                                e8util::vec3 const& o,
                                                e8::intersect_info const& info,
                                                if_scene const* scene,
                                                unsigned depth) const
{
        return 0.0f;
}

std::vector<e8util::vec3>
e8::bidirect_mis_pathtracer::sample(e8util::rng& rng,
                                std::vector<e8util::ray> const& rays,
                                if_scene const* scene,
                                unsigned) const
{
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                e8util::ray const& ray = rays[i];
                e8::intersect_info const& info = scene->intersect(ray);
                if (info.valid) {
                        // compute radiance.
                        e8util::vec3 const& p2_inf = sample_indirect_illum(rng, -ray.v(), info, scene, 0);
                        if (info.light)
                                rad[i] = p2_inf + info.light->emission(-ray.v(), info.normal);
                        else
                                rad[i] = p2_inf;
                }
        }
        return rad;
}
