#ifndef RENDERER_H
#define RENDERER_H

#include <memory>

#include "camera.h"
#include "compositor.h"
#include "frame.h"
#include "pathspace.h"
#include "pathtracer.h"
#include "pathtracerfact.h"
#include "thread.h"

namespace e8 {

/**
 * @brief The pt_image_renderer class Image renderer via CPU path-tracing algorithms.
 */
class pt_image_renderer {
  public:
    /**
     * @brief pt_image_renderer It internally creates multiple path-tracers so they can be run in
     * parallel.
     */
    pt_image_renderer(std::unique_ptr<pathtracer_factory> fact);
    ~pt_image_renderer() = default;

    /**
     * @brief The numerical_stats struct Convergence related statistics.
     */
    struct numerical_stats {
        float sample_sigma;
        float scaled_sigma;
        float max_sigma;
        unsigned num_samples;
        float time_per_sample;
    };

    /**
     * @brief render Computes an estimate of the radiance transported in the path-space and received
     * by the camera.
     * @param path_space All possible ways light paths can be formed.
     * @param light_sources Light sources exist in the path-space.
     * @param cam Camera sensor in the path-space.
     * @param num_samps Total number of samples to gather to form the estimate. However, more
     * samples may be gathered in order to make the computation parallelized better.
     * @param compositor Place where final estimates are stored in the form of image.
     * @return Convergence statistics (see above).
     */
    numerical_stats render(if_path_space const &path_space, if_light_sources const &light_sources,
                           if_camera const &cam, unsigned num_samps, if_compositor *compositor);

  private:
    /**
     * @brief The sampling_task_data struct Sampling configurations.
     */
    struct sampling_task_data : public e8util::if_task_storage {
        sampling_task_data(e8util::data_id_t id, if_path_space const &path_space,
                           if_light_sources const &light_sources,
                           std::vector<e8util::ray> const &rays, unsigned num_samps);

        ~sampling_task_data() override = default;

        if_path_space const &path_space;
        if_light_sources const &light_sources;

        // All rays shooting out from the camera sensor.
        std::vector<e8util::ray> const &rays;

        // Number of samples to compute to form the estimate.
        unsigned num_samps;
    };

    /**
     * @brief The sampling_task class
     */
    class sampling_task : public e8util::if_task {
      public:
        sampling_task();
        sampling_task(sampling_task &&rhs);
        sampling_task(e8::if_pathtracer *pt, unsigned seed);
        ~sampling_task() override;

        sampling_task &operator=(sampling_task rhs);

        void run(e8util::if_task_storage *) override;
        std::vector<e8util::vec3> const &get_estimates() const;

      private:
        std::vector<e8util::vec3> m_estimate;
        e8util::rng m_rng;
        e8::if_pathtracer *m_pt;
    };

    std::vector<sampling_task> m_tasks;
    e8util::thread_pool m_thrpool;

    e8util::rng m_rng;
};

} // namespace e8

#endif // IF_RENDERER_H
