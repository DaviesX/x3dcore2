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

                float cos_w = i.inner(info.normal);
                if (cos_w < 0)
                        continue;

                // evaluate.
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
        // direct.
        e8util::vec3 const& direct = sample_direct_illum(rng, o, info, scene, multi_light_samps);

        static const int mutate_depth = 3;
        float p_survive = 0.5f;
        if (depth >= mutate_depth) {
                if (rng.draw() >= p_survive)
                        return 0.0;
        } else
                p_survive = 1.0f;

        if (depth >= 1)
                multi_indirect_samps = 1;

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

        return direct + multi_indirect/multi_indirect_samps/p_survive;
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
                        rad[i] = sample_indirect_illum(rng, -ray.v(), info, scene, 0, 1, 1);
                        if (info.light)
                                rad[i] += info.light->emission(-ray.v(), info.normal);
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
e8::bidirect_pathtracer::sample_light_illum(e8util::rng& rng,
                                            e8util::vec3 const& rad,
                                            e8util::vec3 const& w,
                                            e8::intersect_info const& poi,
                                            if_scene const* scene,
                                            unsigned depth,
                                            e8::intersect_info& terminate,
                                            e8util::vec3& t) const
{
        static const unsigned mutate_depth = 0;
        float p_survive = 0.5f;
        if (depth >= mutate_depth) {
                if (rng.draw() >= p_survive) {
                        // stop.
                        terminate = poi;
                        t = w;
                        return rad/(1.0f - p_survive);
                }
        } else
                p_survive = 1.0f;

        float mat_pdf;
        e8util::vec3 const& i = poi.mat->sample(rng, poi.normal, w, mat_pdf);
        e8util::ray l(poi.vertex, i);
        e8::intersect_info const& linfo = scene->intersect(l);
        if (linfo.valid) {
                e8util::vec3 const& brdf = poi.mat->eval(poi.normal, w, i);
                float cos_w = poi.normal.inner(i);
                e8util::vec3 const& r = rad*brdf*cos_w/mat_pdf/p_survive;
                return sample_light_illum(rng, r, -i, linfo, scene, depth + 1, terminate, t);
        } else {
                return 0.0f;
        }
}

e8util::vec3
e8::bidirect_pathtracer::sample_illum(e8util::rng& rng,
                                      e8util::vec3 const& o,
                                      e8::intersect_info const& poi,
                                      if_scene const* scene) const
{
        // sample light.
        float light_pdf;
        if_light const* light = scene->sample_light(rng, light_pdf);

        float source_pdf;
        e8util::vec3 p, n, w;
        light->sample(rng, source_pdf, p, n, w);


        e8util::ray light_path(p, w);
        e8::intersect_info const& light_info = scene->intersect(light_path);
        if (!light_info.valid)
                return 0.0f;

        // construct light path.
        e8util::vec3 l = light_info.vertex - p;
        e8util::vec3 illum = light->eval(l, n)/(light_pdf*source_pdf);

        e8::intersect_info terminate;
        e8util::vec3 tray;
        e8util::vec3 const& light_illum = sample_light_illum(rng, illum, -w, light_info, scene, 0, terminate, tray);

        if (light_illum == 0.0f)
                return 0.0f;

        // evaluate.
        e8util::vec3 join_path = poi.vertex - terminate.vertex;
        float distance = join_path.norm();
        join_path = join_path/distance;
        e8util::ray join_ray(terminate.vertex, join_path);
        float t;
        if (!scene->has_intersect(join_ray, 1e-4f, distance - 1e-3f, t)) {
                float cos_w = terminate.normal.inner(join_path);
                e8util::vec3 f1 = light_illum*terminate.mat->eval(terminate.normal, join_path, tray)*cos_w;
                float cos_w2 = o.inner(-join_path);
                e8util::vec3 f2 = poi.mat->eval(poi.normal, o, -join_path)*cos_w2;
                return f1*f2;
        } else
                return 0.0f;
}

e8util::vec3
e8::bidirect_pathtracer::sample_indirect_illum(e8util::rng& rng,
                                                e8util::vec3 const& o,
                                                e8::intersect_info const& info,
                                                if_scene const* scene,
                                                unsigned depth) const
{
        static const unsigned mutate_depth = 0;
        float p_survive = 0.5f;
        if (depth >= mutate_depth) {
                if (rng.draw() >= p_survive) {
                        // Mutate and trace from the other direction.
                        return sample_illum(rng, o, info, scene);
                }
        } else
                p_survive = 1.0f;

        // indirect.
        float mat_pdf;
        e8util::vec3 const& i = info.mat->sample(rng, info.normal, o, mat_pdf);
        e8::intersect_info const& indirect_info = scene->intersect(e8util::ray(info.vertex, i));
        if (indirect_info.valid) {
                e8util::vec3 const& indirect = sample_indirect_illum(rng, i, indirect_info, scene, depth + 1);
                e8util::vec3 const& brdf = info.mat->eval(info.normal, o, i);
                float cos_w = info.normal.inner(i);
                e8util::vec3 r = indirect*brdf*cos_w/mat_pdf/p_survive;
                return r;
        } else
                return 0.0f;
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
                        rad[i] = sample_indirect_illum(rng, -ray.v(), info, scene, 0);
                        if (info.light)
                                rad[i] += info.light->emission(-ray.v(), info.normal);
                }
        }
        return rad;
}
