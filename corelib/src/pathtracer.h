#ifndef IF_PATHTRACER_H
#define IF_PATHTRACER_H

#include "pathspace.h"
#include "tensor.h"
#include <iosfwd>
#include <vector>

namespace e8 {
class if_light;
}
namespace e8 {
class if_light_sources;
}

namespace e8 {

/**
 * @brief The if_path_tracer class CPU path-tracing interface.
 */
class if_path_tracer {
  public:
    if_path_tracer() = default;
    virtual ~if_path_tracer() = default;

    /**
     * @brief The first_hits struct Contains the intersection information about the first hit test.
     */
    struct first_hits {
        first_hits(std::size_t size) : hits(size) {}

        struct hit {
            intersect_info intersect;
            if_light const *light;
        };

        std::vector<hit> hits;
    };

    /**
     * @brief compute_first_hit The first hits are deterministic if we have a infinitestimal apeture
     * size. It will be beneficial to compute the first hits once then re-use across all samples.
     * @param rays The rays that gives the first hits.
     * @param path_space Path space container.
     * @param light_sources Lights container.
     * @return See above.
     */
    static first_hits compute_first_hit(std::vector<e8util::ray> const &rays,
                                        if_path_space const &path_space,
                                        if_light_sources const &light_sources);

    /**
     * @brief sample Compute a single sample of the measurement function estimate.
     * @param rng Random number generator.
     * @param first_hits In aperture configuration, the first hits are deterministic. The
     * path-tracer can save computation by using the cached intersection information about the first
     * hits.
     * @param path_space Defines the set of possible paths.
     * @param light_sources The set of light sources.
     * @return A sample of the measurement function estimate.
     */
    virtual std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                             first_hits const &first_hits,
                                             if_path_space const &path_space,
                                             if_light_sources const &light_sources) const = 0;
};

/**
 * @brief The position_tracer class
 */
class position_tracer : public if_path_tracer {
  public:
    position_tracer() = default;
    ~position_tracer() override = default;

    std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                     first_hits const &first_hits, if_path_space const &path_space,
                                     if_light_sources const &light_sources) const override;
};

/**
 * @brief The normal_tracer class
 */
class normal_tracer : public if_path_tracer {
  public:
    normal_tracer() = default;
    ~normal_tracer() override = default;

    std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                     first_hits const &first_hits, if_path_space const &path_space,
                                     if_light_sources const &light_sources) const override;
};

/**
 * @brief The direct_path_tracer class
 * unidirectional tracer with throughput limited to 2.
 */
class direct_path_tracer : public if_path_tracer {
  public:
    direct_path_tracer() = default;
    ~direct_path_tracer() override = default;

    std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                     first_hits const &first_hits, if_path_space const &path_space,
                                     if_light_sources const &light_sources) const override;
};

/**
 * @brief The unidirect_lt1_path_tracer class
 * unidirectional tracer with unlimited throughput.
 */
class unidirect_path_tracer : public if_path_tracer {
  public:
    unidirect_path_tracer() = default;
    ~unidirect_path_tracer() override = default;

    std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                     first_hits const &first_hits, if_path_space const &path_space,
                                     if_light_sources const &light_sources) const override;

  protected:
    e8util::vec3 sample_indirect_illum(e8util::rng &rng, e8util::vec3 const &o,
                                       e8::intersect_info const &vert,
                                       if_path_space const &path_space,
                                       if_light_sources const &light_sources, unsigned depth) const;
};

/**
 * @brief The unidirect_lt1_path_tracer class
 * unidirectional tracer with unlimited throughput and direct light sampling.
 */
class unidirect_lt1_path_tracer : public if_path_tracer {
  public:
    unidirect_lt1_path_tracer() = default;
    ~unidirect_lt1_path_tracer() override = default;

    std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                     first_hits const &first_hits, if_path_space const &path_space,
                                     if_light_sources const &light_sources) const override;

  protected:
    e8util::vec3 sample_indirect_illum(e8util::rng &rng, e8util::vec3 const &o,
                                       e8::intersect_info const &vert,
                                       if_path_space const &path_space,
                                       if_light_sources const &light_sources, unsigned depth,
                                       unsigned n, unsigned m) const;
};

/**
 * @brief The bidirect_lt2_path_tracer class
 * bidirectional tracer with light throughput limited to 2.
 */
class bidirect_lt2_path_tracer : public if_path_tracer {
  public:
    bidirect_lt2_path_tracer() = default;
    ~bidirect_lt2_path_tracer() override = default;

    std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                     first_hits const &first_hits, if_path_space const &path_space,
                                     if_light_sources const &light_sources) const override;

  protected:
    e8util::vec3 join_with_light_paths(e8util::rng &rng, e8util::vec3 const &o,
                                       e8::intersect_info const &vert,
                                       if_path_space const &path_space,
                                       if_light_sources const &light_sources,
                                       unsigned cam_path_len) const;
    e8util::vec3 sample_indirect_illum(e8util::rng &rng, e8util::vec3 const &o,
                                       e8::intersect_info const &vert,
                                       if_path_space const &path_space,
                                       if_light_sources const &light_sources, unsigned depth) const;
};

/**
 * @brief The bidirect_mis_path_tracer class
 * bidirectional tracer with unlimited throughput and multiple importance
 * sampling over the path space.
 */
class bidirect_mis_path_tracer : public if_path_tracer {
  public:
    bidirect_mis_path_tracer() = default;
    ~bidirect_mis_path_tracer() override = default;

    std::vector<e8util::vec3> sample(e8util::rng &rng, std::vector<e8util::ray> const &rays,
                                     first_hits const &first_hits, if_path_space const &path_space,
                                     if_light_sources const &light_sources) const override;

  protected:
    e8::if_light const *sample_illum_source(e8util::rng &rng, e8util::vec3 &p, e8util::vec3 &n,
                                            e8util::vec3 &w, float &density, float &w_density,
                                            if_light_sources const &light_sources) const;

  private:
    static unsigned const m_max_path_len = 4;
};

} // namespace e8

#endif // IF_PATHTRACER_H
