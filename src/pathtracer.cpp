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

e8::if_light const*
e8::direct_pathtracer::sample_illum_source(e8util::rng& rng,
                                           e8util::vec3& p,
                                           e8util::vec3& n,
                                           float& density,
                                           e8::intersect_info const&,
                                           if_scene const* scene) const
{
        // sample light.
        float light_pdf;
        if_light const* light = scene->sample_light(rng, light_pdf);

        float source_pdf;
        light->sample(rng, source_pdf, p, n);

        density = source_pdf*light_pdf;
        return light;
}

e8util::vec3
e8::direct_pathtracer::transport_illum_source(if_light const* light,
                                              e8util::vec3 const& p_illum,
                                              e8util::vec3 const& n_illum,
                                              e8::intersect_info const& target_vert,
                                              e8util::vec3 const& target_o_ray,
                                              if_scene const* scene) const
{
        // construct light path.
        e8util::vec3 const& l = target_vert.vertex - p_illum;
        e8util::vec3 const& illum = light->eval(l, n_illum);
        if (illum == 0.0f)
                return 0.0f;

        float distance = l.norm();
        e8util::vec3 const& i = -l/distance;

        // evaluate.
        float cos_w = i.inner(target_vert.normal);
        e8util::ray light_ray(target_vert.vertex, i);
        float t;
        if (!scene->has_intersect(light_ray, 1e-4f, distance - 1e-3f, t)) {
                return illum*target_vert.mat->eval(target_vert.normal, target_o_ray, i)*cos_w;
        } else {
                return 0.0f;
        }
}

e8util::vec3
e8::direct_pathtracer::sample_direct_illum(e8util::rng& rng,
                                           e8util::vec3 const& target_o_ray,
                                           e8::intersect_info const& target_vert,
                                           if_scene const* scene,
                                           unsigned multi_light_samps) const
{
        e8util::vec3 rad;
        for (unsigned k = 0; k < multi_light_samps; k ++) {
                e8util::vec3 p, n;
                float density;
                e8::if_light const* light = sample_illum_source(rng, p, n, density, target_vert, scene);
                rad += transport_illum_source(light, p, n, target_vert, target_o_ray, scene)/density;
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

e8::unidirect_pathtracer::unidirect_pathtracer():
        m_ray_mem(new e8util::vec3[m_max_mem]),
        m_vertex_mem(new e8::intersect_info[m_max_mem]),
        m_solid_angle_dens_mem(new float[m_max_mem])
{
}

e8::unidirect_pathtracer::~unidirect_pathtracer()
{
        delete [] m_ray_mem;
        delete [] m_vertex_mem;
        delete [] m_solid_angle_dens_mem;
}

unsigned
e8::unidirect_pathtracer::sample_path(e8util::rng& rng,
                                         e8util::vec3* o,
                                         e8::intersect_info* vertices,
                                         float* dens,
                                         if_scene const* scene,
                                         unsigned depth,
                                         unsigned max_depth) const
{
        if (depth == max_depth - 1)
                return depth;
        float w_dens;
        e8util::vec3 const& i = vertices[depth].mat->sample(rng, vertices[depth].normal, o[depth + 1], w_dens);
        e8::intersect_info const& next_vert = scene->intersect(e8util::ray(vertices[depth].vertex, i));
        if (next_vert.valid) {
                o[depth + 1] = i;
                dens[depth + 1] = w_dens;
                vertices[depth + 1] = next_vert;
                return sample_path(rng, o, vertices, dens, scene, depth + 1, max_depth);
        } else {
                return depth;
        }
}

unsigned
e8::unidirect_pathtracer::sample_path(e8util::rng& rng,
                                      e8util::vec3* o,
                                      e8::intersect_info* vertices,
                                      float* dens,
                                      e8util::ray const& r0,
                                      float dens0,
                                      if_scene const* scene,
                                      unsigned max_depth) const
{
        e8::intersect_info const& vert0 = scene->intersect(r0);
        if (!vert0.valid) {
                return 0;
        } else {
                o[0] = r0.o();
                vertices[0] = vert0;
                dens[0] = dens0;
                return 1 + sample_path(rng, o, vertices, dens, scene, 0, max_depth);
        }
}

e8util::vec3
e8::unidirect_pathtracer::transport_subpath(e8util::vec3 const& src_rad,
                                            e8util::vec3 const& appending_ray,
                                            e8util::vec3 const* o,
                                            e8::intersect_info const* vertices,
                                            float const* dens,
                                            unsigned sub_path_len,
                                            bool is_forward) const
{

        if (is_forward) {
                e8util::vec3 transport = src_rad;
                for (unsigned k = 0; k < sub_path_len - 1; k ++) {
                        transport *= vertices[k].mat->eval(vertices[k].normal, o[k + 1], o[k])*
                                        vertices[k].normal.inner(o[k])/
                                        dens[k];
                }
                return transport*vertices[sub_path_len - 1].
                                mat->eval(vertices[sub_path_len - 1].normal,
                                          appending_ray,
                                          o[sub_path_len - 1])*
                                vertices[sub_path_len - 1].normal.inner(o[sub_path_len - 1])/
                                dens[sub_path_len - 1];
        } else {
                e8util::vec3 transport = src_rad*vertices[sub_path_len - 1].
                                mat->eval(vertices[sub_path_len - 1].normal,
                                          o[sub_path_len - 1],
                                          appending_ray)*
                                vertices[sub_path_len - 1].normal.inner(appending_ray)/
                                dens[sub_path_len - 1];
                for (int k = static_cast<int>(sub_path_len) - 2; k >= 0; k --) {
                        transport *= vertices[k].mat->eval(vertices[k].normal, o[k], o[k + 1])*
                                        vertices[k].normal.inner(o[k + 1])/
                                        dens[k];
                }
                return transport;
        }
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

e8::bidirect_lt2_pathtracer::bidirect_lt2_pathtracer()
{
}

e8::bidirect_lt2_pathtracer::~bidirect_lt2_pathtracer()
{
}

e8util::vec3
e8::bidirect_lt2_pathtracer::join_with_light_paths(e8util::rng& rng,
                                                   e8util::vec3 const& o,
                                                   e8::intersect_info const& poi,
                                                   if_scene const* scene,
                                                   unsigned cam_path_len) const
{
        e8util::vec3 const& p1_direct = sample_direct_illum(rng, o, poi, scene, 1);

        // sample light.
        float light_pdf, source_p_pdf, source_w_pdf;
        if_light const* light = scene->sample_light(rng, light_pdf);
        e8util::vec3 p, n, w;
        light->sample(rng, source_p_pdf, source_w_pdf, p, n, w);
        e8util::ray light_path(p, w);
        e8::intersect_info const& light_info = scene->intersect(light_path);
        if (!light_info.valid)
                return 0.0f;

        // construct light path.
        e8util::vec3 const& illum = light->emission(w, n)/(light_pdf*source_p_pdf*source_w_pdf);

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
        if (cos_wo > 0.0f &&
                        cos_wi > 0.0f &&
                        cos_w2 > 0.0f &&
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
e8::bidirect_lt2_pathtracer::sample_indirect_illum(e8util::rng& rng,
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
e8::bidirect_lt2_pathtracer::sample(e8util::rng& rng,
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

e8::if_light const*
e8::bidirect_mis_pathtracer::sample_illum_source(e8util::rng& rng,
                                                 e8util::vec3& p,
                                                 e8util::vec3& n,
                                                 e8util::vec3& w,
                                                 float& density,
                                                 float& w_density,
                                                 if_scene const* scene) const
{
        // sample light.
        float light_dens;
        if_light const* light = scene->sample_light(rng, light_dens);

        float source_dens;
        light->sample(rng, source_dens, w_density, p, n, w);

        density = source_dens*light_dens;
        return light;
}


e8util::vec3
e8::bidirect_mis_pathtracer::sample_all_subpaths(e8util::vec3 const* cam_o_rays,
                                                 e8::intersect_info const* cam_vertices,
                                                 float* cam_dens,
                                                 unsigned cam_path_len,
                                                 e8util::vec3 const* light_o_rays,
                                                 e8::intersect_info const* light_vertices,
                                                 float* light_dens,
                                                 unsigned light_path_len,
                                                 e8util::vec3 const& light_p,
                                                 e8util::vec3 const& light_n,
                                                 float pdf_light_w,
                                                 if_light const* light,
                                                 if_scene const* scene) const
{
        e8util::vec3 rad;
        for (unsigned i = 0; i < cam_path_len; i ++) {
                for (unsigned j = 0; j < light_path_len; j ++) {
                        e8util::vec3 path_rad;
                        if (j == 0) {
                                // direct light sampling.
                                float dens = light_dens[0]/pdf_light_w;         // direction was not chosen by random process.
                                e8util::vec3 transported_light_illum = transport_illum_source(light,
                                                                                              light_p,
                                                                                              light_n,
                                                                                              cam_vertices[i],
                                                                                              cam_o_rays[i],
                                                                                              scene)/dens;
                                // compute light transportation for camera subpath.
                                e8util::vec3 join_path = (cam_vertices[i].vertex - light_p).normalize();
                                path_rad = transport_subpath(transported_light_illum,
                                                             join_path,
                                                             cam_o_rays,
                                                             cam_vertices,
                                                             cam_dens,
                                                             i + 1,
                                                             false);
                        } else {
                                e8util::vec3 join_path = cam_vertices[i].vertex - light_vertices[j].vertex;
                                float distance = join_path.norm();
                                join_path = join_path/distance;
                                e8util::ray join_ray(cam_vertices[i].vertex, join_path);
                                float cos_w2 = light_vertices[j].normal.inner(-light_o_rays[j - 1]);
                                float cos_wo = light_vertices[j].normal.inner(join_path);
                                float cos_wi = cam_vertices[i].normal.inner(-join_path);
                                float t;
                                if (cos_wo <= 0.0f ||
                                                cos_wi <= 0.0f ||
                                                cos_w2 <= 0.0f ||
                                                scene->has_intersect(join_ray, 1e-4f, distance - 1e-3f, t)) {
                                        // invisible join path.
                                        continue;
                                } else {
                                        // compute light transportation for light subpath.
                                        e8util::vec3 light_illum = light->emission(-light_o_rays[0], light_n)/light_dens[0];
                                        e8util::vec3 light_subpath_rad = transport_subpath(light_illum,
                                                                                           -join_path,
                                                                                           light_o_rays,
                                                                                           light_vertices,
                                                                                           light_dens,
                                                                                           j + 1,
                                                                                           true);

                                        // transport light for the join path.
                                        e8util::vec3 transported_light_illum = light_subpath_rad*cos_wo/(distance*distance);

                                        // compute light transportation for camera subpath.
                                        path_rad = transport_subpath(transported_light_illum,
                                                                     join_path,
                                                                     cam_o_rays,
                                                                     cam_vertices,
                                                                     cam_dens,
                                                                     i + 1,
                                                                     false);
                                }
                                rad += path_rad/(i + j + 1);
                        }
                }
        }
        return rad;
}

std::vector<e8util::vec3>
e8::bidirect_mis_pathtracer::sample(e8util::rng& rng,
                                    std::vector<e8util::ray> const& rays,
                                    if_scene const* scene,
                                    unsigned) const
{
        std::vector<e8util::vec3> rad(rays.size());
        for (unsigned i = 0; i < rays.size(); i ++) {
                // initialize the first paths for both camera and light.
                e8util::ray const& cam_path0 = rays[i];
                e8util::vec3 light_p;
                e8util::vec3 light_n;
                e8util::vec3 light_w;
                float light_dens;
                float light_w_dens;
                if_light const* light = sample_illum_source(rng,
                                                            light_p,
                                                            light_n,
                                                            light_w,
                                                            light_dens,
                                                            light_w_dens,
                                                            scene);
                e8util::ray const& light_path0 = e8util::ray(light_p, light_w);

                // produce both camera and light paths.
                unsigned cam_path_len = sample_path(rng,
                                                    &m_ray_mem[0],
                                                    &m_vertex_mem[0],
                                                    &m_solid_angle_dens_mem[0],
                                                    cam_path0,
                                                    1.0f,
                                                    scene,
                                                    m_max_mem/2);
                unsigned light_path_len = sample_path(rng,
                                                      &m_ray_mem[m_max_mem/2],
                                                      &m_vertex_mem[m_max_mem/2],
                                                      &m_solid_angle_dens_mem[m_max_mem/2],
                                                      light_path0,
                                                      light_dens,
                                                      scene,
                                                      m_max_mem/2);

                // compute radiance for different strategies.
                rad[i] = sample_all_subpaths(&m_ray_mem[0],
                                             &m_vertex_mem[0],
                                             &m_solid_angle_dens_mem[0],
                                             cam_path_len,
                                             &m_ray_mem[m_max_mem/2],
                                             &m_vertex_mem[m_max_mem/2],
                                             &m_solid_angle_dens_mem[m_max_mem/2],
                                             light_path_len,
                                             light_p,
                                             light_n,
                                             light_w_dens,
                                             light,
                                             scene);
        }
        return rad;
}
