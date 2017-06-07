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
}

void
e8::ol_image_renderer::render(if_scene const& scene, if_camera const& cam, if_frame& frame)
{
        e8util::mat44 const& proj = cam.projection();
        if (proj != m_t || frame.width() != m_w || frame.height() != m_h) {
                m_t = proj;
                m_w = frame.width();
                m_h = frame.height();
                m_rays = cam.sample(0, 0, m_w, m_h, 1);
                m_rad.resize(m_w*m_h);
                m_samps = 0;
                for (unsigned i = 0; i < m_w*m_h; i ++)
                        m_rad[i] = 0.0f;
        }
        std::vector<e8util::vec3> const& samples = m_pt->sample(m_rays, scene, 10);
        m_samps += 10;
        float pr = 1.0f/m_samps;
        for (unsigned j = 0; j < m_h; j ++) {
                for (unsigned i = 0; i < m_w; i ++) {
                        unsigned p = i + j*m_w;
                        m_rad[p] = m_rad[p] + samples[p];
                        e8util::vec3 const& r = pr*m_rad[p];
                        frame(i, j) = e8util::vec4({r(0), r(1), r(2), 1.0f});
                }
        }
        frame.commit();
}

e8::rendering_stats
e8::ol_image_renderer::get_stats() const
{
}
