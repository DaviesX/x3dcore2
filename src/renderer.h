#ifndef RENDERER_H
#define RENDERER_H

#include "thread.h"
#include "scene.h"
#include "camera.h"
#include "compositor.h"
#include "frame.h"
#include "pathtracer.h"
#include "pathtracerfact.h"


namespace e8
{

struct rendering_stats
{
        float           sample_sigma;
        float           scaled_sigma;
        float           max_sigma;
        unsigned        num_samples;
        float           time_per_sample;
};

class if_im_renderer
{
public:
        if_im_renderer();
        virtual ~if_im_renderer();

        virtual void                    render(if_scene const* scene, if_camera const* cam, if_compositor* compositor) = 0;
        virtual rendering_stats         get_stats() const = 0;
        bool                            update_image_view(if_camera const* cam, if_compositor* compositor);
protected:
        unsigned                        m_w;
        unsigned                        m_h;
        e8util::mat44                   m_t;
};


class pt_image_renderer: public if_im_renderer
{
public:
        pt_image_renderer(pathtracer_factory* fact);
        ~pt_image_renderer() override;

        void                    render(if_scene const* scene, if_camera const* cam, if_compositor* compositor) override;
        rendering_stats         get_stats() const override;
private:
        struct sampling_task_data
        {
                sampling_task_data():
                        scene(nullptr)
                {}

                sampling_task_data(if_scene const* scene,
                                   std::vector<e8util::ray> const& rays):
                        scene(scene),
                        rays(rays)
                {}

                if_scene const*                 scene;
                std::vector<e8util::ray>        rays;
        };

        class sampling_task: public e8util::if_task
        {
        public:
                sampling_task():
                        m_pt(nullptr)
                {}

                sampling_task(e8::if_pathtracer* pt);
                ~sampling_task() override;

                void                            run(void*) override;
                std::vector<e8util::vec3>       get_estimates() const;
        private:
                std::vector<e8util::vec3>               m_estimate;
                e8util::rng                             m_rng;
                e8::if_pathtracer*                      m_pt;
        };

        unsigned                        m_num_tiles_per_dim;
        unsigned                        m_num_tasks;
        sampling_task*                  m_tasks;
        sampling_task_data*             m_task_storages;
        e8util::task_info*              m_task_infos;
        e8util::thread_pool*            m_thrpool;
        pathtracer_factory*             m_fact;

        e8util::rng                     m_rng;
        std::vector<e8util::vec3>       m_rad;
        unsigned                        m_samps;
};

}

#endif // IF_RENDERER_H
