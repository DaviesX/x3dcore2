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
                } else {
                        rad[i] = 0.0f;
                }
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
                } else {
                        rad[i] = 0.0f;
                }
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
                        if (info.light)
                                f = f + info.light->emission();
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
                }
        }
        return rad;
}
