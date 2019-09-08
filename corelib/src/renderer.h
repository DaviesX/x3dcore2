#ifndef RENDERER_H
#define RENDERER_H

#include "camera.h"
#include "compositor.h"
#include "frame.h"
#include "pathspace.h"
#include "pathtracer.h"
#include "pathtracerfact.h"
#include "thread.h"

namespace e8 {

struct rendering_stats
{
    float sample_sigma;
    float scaled_sigma;
    float max_sigma;
    unsigned num_samples;
    float time_per_sample;
};

class pt_image_renderer
{
public:
    pt_image_renderer(pathtracer_factory *fact);
    ~pt_image_renderer();

    bool update_image_view(if_camera const &cam, if_compositor *compositor);
    void render(if_path_space const &path_space,
                if_light_sources const &light_sources,
                if_camera const &cam,
                if_compositor *compositor);
    rendering_stats get_stats() const;

private:
    struct sampling_task_data : public e8util::if_task_storage
    {
        sampling_task_data();
        sampling_task_data(e8util::data_id_t id,
                           if_path_space const *path_space,
                           if_light_sources const *light_sources,
                           std::vector<e8util::ray> const &rays);

        virtual ~sampling_task_data();

        if_path_space const *path_space;
        if_light_sources const *light_sources;
        std::vector<e8util::ray> rays;
    };

    class sampling_task : public e8util::if_task
    {
    public:
        sampling_task();
        sampling_task(sampling_task &&rhs);
        sampling_task(e8::if_pathtracer *pt);
        ~sampling_task() override;

        sampling_task &operator=(sampling_task rhs);
        void run(e8util::if_task_storage *) override;
        std::vector<e8util::vec3> get_estimates() const;

    private:
        std::vector<e8util::vec3> m_estimate;
        e8util::rng m_rng;
        e8::if_pathtracer *m_pt;
    };

    unsigned m_w;
    unsigned m_h;
    e8util::mat44 m_t;

    unsigned m_num_tiles_per_dim;
    unsigned m_num_tasks;
    sampling_task *m_tasks;
    sampling_task_data *m_task_storages;
    e8util::thread_pool *m_thrpool;
    pathtracer_factory *m_fact;

    e8util::rng m_rng;
    std::vector<e8util::vec3> m_rad;
    unsigned m_samps;
};

} // namespace e8

#endif // IF_RENDERER_H
