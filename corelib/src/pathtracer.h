#ifndef IF_PATHTRACER_H
#define IF_PATHTRACER_H

#include <vector>
#include "tensor.h"
#include "pathspace.h"
#include "lightsources.h"

namespace e8 {

/**
 * @brief The if_pathtracer class
 */
class if_pathtracer
{
public:
        if_pathtracer();
        virtual ~if_pathtracer();

        virtual std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                                       std::vector<e8util::ray> const& rays,
                                                       if_path_space const& path_space,
                                                       if_light_sources const& light_sources,
                                                       unsigned n) const = 0;
};

/**
 * @brief The position_pathtracer class
 */
class position_pathtracer: public if_pathtracer
{
public:
        position_pathtracer();
        ~position_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_path_space const& path_space,
                                               if_light_sources const& light_sources,
                                               unsigned n) const override;
};

/**
 * @brief The normal_pathtracer class
 */
class normal_pathtracer: public if_pathtracer
{
public:
        normal_pathtracer();
        ~normal_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_path_space const& path_space,
                                               if_light_sources const& light_sources,
                                               unsigned n) const override;
};

/**
 * @brief The direct_pathtracer class
 * unidirectional tracer with throughput limited to 2.
 */
class direct_pathtracer: public if_pathtracer
{
public:
        direct_pathtracer();
        ~direct_pathtracer() override;

        virtual std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                                       std::vector<e8util::ray> const& rays,
                                                       if_path_space const& path_space,
                                                       if_light_sources const& light_sources,
                                                       unsigned n) const override;
protected:
        if_light const*                         sample_illum_source(e8util::rng& rng,
                                                                    e8util::vec3& p,
                                                                    e8util::vec3& n,
                                                                    float& density,
                                                                    e8::intersect_info const& target_vert,
                                                                    if_light_sources const& light_sources) const;
        e8util::vec3                            transport_illum_source(if_light const& light,
                                                                       e8util::vec3 const& p_illum,
                                                                       e8util::vec3 const& n_illum,
                                                                       e8::intersect_info const& target_vert,
                                                                       e8util::vec3 const& target_o_ray,
                                                                       if_path_space const& path_space) const;
        e8util::vec3                            sample_direct_illum(e8util::rng& rng,
                                                                    e8util::vec3 const& target_o_ray,
                                                                    e8::intersect_info const& target_vert,
                                                                    if_path_space const& path_space,
                                                                    if_light_sources const& light_sources,
                                                                    unsigned n) const;
};

/**
 * @brief The unidirect_pathtracer class
 * unidirectional tracer with unlimited throughput and direct light sampling.
 */
class unidirect_pathtracer: public direct_pathtracer
{
public:
        unidirect_pathtracer();
        ~unidirect_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_path_space const& path_space,
                                               if_light_sources const& light_sources,
                                               unsigned n) const override;
protected:
        struct sampled_pathlet
        {
                e8util::vec3 o;
                e8::intersect_info vert;
                float dens;

                sampled_pathlet()
                {
                }

                sampled_pathlet(e8util::vec3 o,
                                e8::intersect_info vert,
                                float dens):
                        o(o),
                        vert(vert),
                        dens(dens)
                {
                }
        };

        unsigned                        sample_path(e8util::rng& rng,
                                                    sampled_pathlet* sampled_path,
                                                    e8util::ray const& r0,
                                                    float dens0,
                                                    if_path_space const& path_space,
                                                    unsigned max_depth) const;
        e8util::vec3                    transport_subpath(e8util::vec3 const& src_rad,
                                                          e8util::vec3 const& appending_ray,
                                                          float appending_ray_dens,
                                                          sampled_pathlet const* sampled_path,
                                                          unsigned sub_path_len,
                                                          bool is_forward) const;
        float                           subpath_density(float src_dens,
                                                        sampled_pathlet const* sampled_path,
                                                        unsigned path_start,
                                                        unsigned path_end) const;
        e8util::vec3                    sample_indirect_illum(e8util::rng& rng,
                                                              e8util::vec3 const& o,
                                                              e8::intersect_info const& vert,
                                                              if_path_space const& path_space,
                                                              if_light_sources const& light_sources,
                                                              unsigned depth,
                                                              unsigned n,
                                                              unsigned m) const;
private:
        unsigned                        sample_path(e8util::rng& rng,
                                                    sampled_pathlet* sampled_path,
                                                    if_path_space const& path_space,
                                                    unsigned depth,
                                                    unsigned max_depth) const;

        static unsigned const           m_max_path_len = 4;
};

/**
 * @brief The bidirect_lt2_pathtracer class
 * bidirectional tracer with light throughput limited to 2.
 */
class bidirect_lt2_pathtracer: public unidirect_pathtracer
{
public:
        bidirect_lt2_pathtracer();
        ~bidirect_lt2_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_path_space const& path_space,
                                               if_light_sources const& light_sources,
                                               unsigned n) const override;
protected:
        e8util::vec3                    join_with_light_paths(e8util::rng& rng,
                                                              e8util::vec3 const& o,
                                                              e8::intersect_info const& vert,
                                                              if_path_space const& path_space,
                                                              if_light_sources const& light_sources,
                                                              unsigned cam_path_len) const;
        e8util::vec3                    sample_indirect_illum(e8util::rng& rng,
                                                              e8util::vec3 const& o,
                                                              e8::intersect_info const& vert,
                                                              if_path_space const& path_space,
                                                              if_light_sources const& light_sources,
                                                              unsigned depth) const;
};

/**
 * @brief The bidirect_mis_pathtracer class
 * bidirectional tracer with unlimited throughput and multiple importance sampling over the path space.
 */
class bidirect_mis_pathtracer: public unidirect_pathtracer
{
public:
        bidirect_mis_pathtracer();
        ~bidirect_mis_pathtracer() override;

        std::vector<e8util::vec3>       sample(e8util::rng& rng,
                                               std::vector<e8util::ray> const& rays,
                                               if_path_space const& path_space,
                                               if_light_sources const& light_sources,
                                               unsigned n) const override;
protected:
        e8::if_light const*             sample_illum_source(e8util::rng& rng,
                                                            e8util::vec3& p,
                                                            e8util::vec3& n,
                                                            e8util::vec3& w,
                                                            float& density,
                                                            float& w_density,
                                                            if_light_sources const& light_sources) const;
        e8util::vec3                    sample_all_subpaths(sampled_pathlet const* cam_path,
                                                            unsigned cam_path_len,
                                                            sampled_pathlet const* light_path,
                                                            unsigned light_path_len,
                                                            e8util::vec3 const& light_p,
                                                            e8util::vec3 const& light_n,
                                                            float pdf_light_p_dens,
                                                            if_light const& light,
                                                            if_path_space const& path_space) const;
private:
        static unsigned const           m_max_path_len = 4;
};

}


#endif // IF_PATHTRACER_H
