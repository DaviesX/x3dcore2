#include "renderer.h"


e8::if_im_renderer::if_im_renderer()
{
}

e8::if_im_renderer::~if_im_renderer()
{
}



e8::ol_image_renderer::ol_image_renderer(if_pathtracer* pt):
        m_pt(pt)
{
}

e8::ol_image_renderer::~ol_image_renderer()
{
        delete m_pt;
}

void
e8::ol_image_renderer::render(if_scene const* scene, if_camera const* cam, if_compositor* compositor)
{
        e8util::rng rng(100);

        // generate camera seed ray.
        e8util::mat44 const& proj = cam->projection();
        if (proj != m_t || compositor->width() != m_w || compositor->height() != m_h) {
                m_t = proj;
                m_w = compositor->width();
                m_h = compositor->height();
                m_rad.resize(m_w*m_h);
                m_rays.resize(m_w*m_h);
                m_samps = 0;
                for (unsigned j = 0; j < m_h; j ++) {
                        for (unsigned i = 0; i < m_w; i ++) {
                                float pdf;
                                m_rays[i + j*m_w] = cam->sample(rng, i, j, m_w, m_h, pdf);
                                m_rad[i + j*m_w] = 0.0f;
                        }
                }
        }

        unsigned const n_samples = 20;
        std::vector<e8util::vec3> const& estimate = m_pt->sample(rng, m_rays, scene, n_samples);

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
e8::ol_image_renderer::get_stats() const
{
        throw std::string("Not implemented yet");
}
