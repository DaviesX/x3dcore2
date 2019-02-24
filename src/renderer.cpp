#include "thread.h"
#include "renderer.h"


e8::if_im_renderer::if_im_renderer()
{
}

e8::if_im_renderer::~if_im_renderer()
{
}

bool
e8::if_im_renderer::update_image_view(if_camera const* cam, if_compositor* compositor)
{
        e8util::mat44 const& proj = cam->projection();
        if (proj != m_t || compositor->width() != m_w || compositor->height() != m_h) {
                m_t = proj;
                m_w = compositor->width();
                m_h = compositor->height();
                return true;
        } else {
                return false;
        }
}


e8::pt_image_renderer::pt_image_renderer(if_pathtracer* pt):
        m_pt(pt), m_rng(100), m_thrpool(e8util::cpu_core_count())
{
        m_ncores = e8util::cpu_core_count();
        m_ray_parts = new std::vector<e8util::ray> [m_ncores];
        for (unsigned i = 0; i < m_ncores; i ++) {
                m_ray_parts[i] = std::vector<e8util::ray>();
        }
}

e8::pt_image_renderer::~pt_image_renderer()
{
        delete m_pt;
        delete [] m_ray_parts;
}

class sampling_task: public e8util::if_task
{
public:
        sampling_task(e8::if_pathtracer const* pt,
                      std::vector<e8util::ray> const& rays,
                      e8::if_scene const* scene):
                m_pt(pt), m_rays(rays), m_scene(scene)
        {
        }

        void    run(void*) override;

private:
        e8util::rng                             m_rng;
        e8::if_pathtracer const*                m_pt;
        std::vector<e8util::ray> const&         m_rays;
        e8::if_scene const*                     m_scene;
        std::vector<e8util::vec3>               m_estimate;
};

void
sampling_task::run(void*)
{
        m_estimate = m_pt->sample(m_rng, m_rays, m_scene, 1);
}

void
e8::pt_image_renderer::render(if_scene const* scene, if_camera const* cam, if_compositor* compositor)
{
        // generate camera seed ray.
        if (update_image_view(cam, compositor)) {
                m_rad.resize(m_w*m_h);
                m_rays.resize(m_w*m_h);
                m_samps = 0;
                for (unsigned j = 0; j < m_h; j ++) {
                        for (unsigned i = 0; i < m_w; i ++) {
                                float pdf;
                                m_rays[i + j*m_w] = cam->sample(m_rng, i, j, m_w, m_h, pdf);
                                m_rad[i + j*m_w] = 0.0f;
                        }
                }
        }

        unsigned const n_samples = 1;
        std::vector<e8util::vec3> const& estimate = m_pt->sample(m_rng, m_rays, scene, n_samples);

        // accumulate result.
        m_samps += 1;
        float pr = 1.0f/m_samps;
        for (unsigned j = 0; j < m_h; j ++) {
                for (unsigned i = 0; i < m_w; i ++) {
                        unsigned p = i + j*m_w;
                        m_rad[p] = m_rad[p] + estimate[p];
                        e8util::vec3 const& r = pr*m_rad[p];
                        (*compositor)(i, j) = r.homo(1.0f);
                }
        }
}

e8::rendering_stats
e8::pt_image_renderer::get_stats() const
{
        throw std::string("Not implemented yet");
}
