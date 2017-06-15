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
e8::direct_pathtracer::sample_direct_illum(e8util::rng& rng, e8util::vec3 const& o, e8::intersect_info const& info, if_scene const* scene, unsigned n) const
{
        e8util::vec3 rad;
        for (unsigned k = 0; k < n; k ++) {
                // sample light.
                float light_pdf;
                if_light const* light = scene->sample_light(rng, light_pdf);

                float source_pdf;
                e8util::vec3 p, n;
                light->sample(rng, source_pdf, p, n);

                // construct light path.
                e8util::vec3 const& l = info.vertex - p;
                e8util::vec3 const& illum = light->eval(l, n);
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
                if (!scene->has_intersect(light_ray, 1e-4f, distance - 1e-3f, t)) {
                        e8util::vec3 f = illum*info.mat->eval(info.normal, o, i)*cos_w;
                        rad = rad + f/(light_pdf*source_pdf);
                }
        }
        return rad/n;
}

std::vector<e8util::vec3>
e8::direct_pathtracer::sample(e8util::rng& rng, std::vector<e8util::ray> const& rays, if_scene const* scene, unsigned n) const
{
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                e8util::ray const& ray = rays[i];
                e8::intersect_info const& info = scene->intersect(ray);
                if (info.valid) {
                        // compute radiance.
                        rad[i] = sample_direct_illum(rng, -ray.v(), info, scene, n);
                        if (info.light)
                                rad[i] = rad[i] + info.light->emission(-ray.v(), info.normal);
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
                                                unsigned n,
                                                unsigned m) const
{
        // direct.
        e8util::vec3 const& ld = sample_direct_illum(rng, o, info, scene, n);

        static const int mutate_depth = 3;
        float p_survive = 0.5f;
        if (depth >= mutate_depth) {
                if (rng.draw() >= p_survive)
                        return 0.0;
        } else
                p_survive = 1.0f;

        if (depth >= 1)
                m = 1;

        // indirect.
        float mat_pdf;
        e8util::vec3 r;
        for (unsigned k = 0; k < m; k ++) {
                e8util::vec3 const& i = info.mat->sample(rng, info.normal, o, mat_pdf);
                e8::intersect_info const& indirect_info = scene->intersect(e8util::ray(info.vertex, i));
                if (indirect_info.valid) {
                       e8util::vec3 const& li = sample_indirect_illum(rng, i, indirect_info, scene, depth + 1, n, m);
                       r = r + li*info.mat->eval(info.normal, o, i)*info.normal.inner(i)/mat_pdf;
                }
        }

        return ld + r/m/p_survive;
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
                        rad[i] = sample_indirect_illum(rng, -ray.v(), info, scene, 0, 2, 1);
                        if (info.light)
                                rad[i] = rad[i] + info.light->emission(-ray.v(), info.normal);
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
                                            e8::intersect_info const& info,
                                            if_scene const* scene,
                                            unsigned depth,
                                            e8::intersect_info& terminate,
                                            e8util::vec3& t) const
{
        static const unsigned mutate_depth = 1;
        float p_survive = 0.5f;
        if (depth >= mutate_depth) {
                if (rng.draw() >= p_survive) {
                        // stop.
                        terminate = info;
                        t = w;
                        return rad/(1.0f - p_survive);
                }
        } else
                p_survive = 1.0f;

        float mat_pdf;
        e8util::vec3 const& i = info.mat->sample(rng, info.normal, w, mat_pdf);
        e8util::ray l(info.vertex, i);
        e8::intersect_info const& linfo = scene->intersect(l);
        if (linfo.valid) {
                e8util::vec3 const& r = rad*info.mat->eval(info.normal, w, i)*info.normal.inner(i)/mat_pdf/p_survive;
                return sample_light_illum(rng, r, -i, linfo, scene, depth + 1, terminate, t);
        } else {
                return 0.0f;
        }
}

e8util::vec3
e8::bidirect_pathtracer::sample_illum(e8util::rng& rng, e8util::vec3 const& o, e8::intersect_info const& info,
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

        e8::intersect_info terminate;
        e8util::vec3 tray;
        e8util::vec3 const& illum = sample_light_illum(rng, light->emission(w, n), -w, light_info, scene, 0,
                                                       terminate, tray);

        if (illum == 0.0f)
                return 0.0f;

        // construct light path.
        e8util::vec3 l = info.vertex - terminate.vertex;
        float distance = l.norm();
        l = l/distance;
        float cos_w = l.inner(terminate.normal);
        if (cos_w < 0)
                return 0.0f;

        // evaluate.
        e8util::ray light_ray(terminate.vertex, l);
        float t;
        if (!scene->has_intersect(light_ray, 1e-4f, distance - 1e-3f, t)) {
                e8util::vec3 f = illum*terminate.mat->eval(terminate.normal, tray, l)*cos_w;
                f = f/(light_pdf*source_pdf);
                return f*info.mat->eval(info.normal, o, -l)*o.inner(-l);
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
        static const unsigned mutate_depth = 1;
        float p_survive = 0.5f;
        if (depth >= mutate_depth) {
                if (rng.draw() >= p_survive) {
                        // Trace from other direction.
                        return sample_illum(rng, o, info, scene)/(1.0f - p_survive);
                }
        } else
                p_survive = 1.0f;

        // indirect.
        float mat_pdf;
        e8util::vec3 const& i = info.mat->sample(rng, info.normal, o, mat_pdf);
        e8::intersect_info const& indirect_info = scene->intersect(e8util::ray(info.vertex, i));
        if (indirect_info.valid) {
                e8util::vec3 const& li = sample_indirect_illum(rng, i, indirect_info, scene, depth + 1);
                e8util::vec3 r = li*info.mat->eval(info.normal, o, i)*info.normal.inner(i)/mat_pdf/p_survive;
                if (info.light)
                        r = r + info.light->emission(o, info.normal);
                return r;
        } else {
                return sample_illum(rng, o, info, scene)/p_survive;
        }
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
                }
        }
        return rad;
}
