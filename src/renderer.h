#ifndef RENDERER_H
#define RENDERER_H

#include "thread.h"
#include "scene.h"
#include "camera.h"
#include "compositor.h"
#include "frame.h"
#include "pathtracer.h"


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
        pt_image_renderer(if_pathtracer* pt);
        ~pt_image_renderer() override;

        void                    render(if_scene const* scene, if_camera const* cam, if_compositor* compositor) override;
        rendering_stats         get_stats() const override;
private:
        unsigned                        m_ncores;
        if_pathtracer*                  m_pt;
        std::vector<e8util::ray>        m_rays;
        std::vector<e8util::ray>*       m_ray_parts;
        std::vector<e8util::vec3>       m_rad;
        unsigned                        m_samps;

        e8util::rng                     m_rng;
        e8util::thread_pool             m_thrpool;
};

}

#endif // IF_RENDERER_H
