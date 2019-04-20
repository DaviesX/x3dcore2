#include <cmath>
#include "thread.h"
#include "tensor.h"
#include "renderer.h"


e8::if_im_renderer::if_im_renderer():
        m_w(0),
        m_h(0),
        m_t(e8util::mat44_scale(1.0f))
{
}

e8::if_im_renderer::~if_im_renderer()
{
}

bool
e8::if_im_renderer::update_image_view(if_camera const* cam, if_compositor* compositor)
{
        e8util::mat44 const& proj = cam->projection();
        if (proj != m_t ||
            compositor->width() != m_w ||
            compositor->height() != m_h) {
                m_t = proj;
                m_w = compositor->width();
                m_h = compositor->height();
                return true;
        } else {
                return false;
        }
}




e8::pt_image_renderer::sampling_task_data::sampling_task_data():
        scene(nullptr)
{
}

e8::pt_image_renderer::sampling_task_data::sampling_task_data(e8util::data_id_t id,
                                                              if_path_space const* scene,
                                                              std::vector<e8util::ray> const& rays):
        e8util::if_task_storage(id),
        scene(scene),
        rays(rays)
{
}

e8::pt_image_renderer::sampling_task_data::~sampling_task_data()
{
}

e8::pt_image_renderer::sampling_task::sampling_task():
        e8util::if_task(false),
        m_pt(nullptr)
{
}

e8::pt_image_renderer::sampling_task::sampling_task(e8::if_pathtracer* pt):
        e8util::if_task(false),
        m_rng(100),
        m_pt(pt)
{
}

e8::pt_image_renderer::sampling_task::sampling_task(sampling_task&& rhs)
{
        m_estimate = rhs.m_estimate;
        m_rng = rhs.m_rng;
        m_pt = rhs.m_pt;
        rhs.m_pt = nullptr;
}

e8::pt_image_renderer::sampling_task::~sampling_task()
{
        delete m_pt;
}

e8::pt_image_renderer::sampling_task&
e8::pt_image_renderer::sampling_task::operator=(sampling_task rhs)
{
        m_estimate = rhs.m_estimate;
        m_rng = rhs.m_rng;
        std::swap(m_pt, rhs.m_pt);
        return *this;
}

void
e8::pt_image_renderer::sampling_task::run(e8util::if_task_storage* p)
{
        unsigned const n_samples = 1;
        sampling_task_data* data = static_cast<sampling_task_data*>(p);
        m_estimate = m_pt->sample(m_rng, data->rays, data->scene, n_samples);
}

std::vector<e8util::vec3>
e8::pt_image_renderer::sampling_task::get_estimates() const
{
        return m_estimate;
}


e8::pt_image_renderer::pt_image_renderer(pathtracer_factory* fact):
        m_num_tiles_per_dim(static_cast<unsigned>(std::ceil(std::sqrt(e8util::cpu_core_count())))),
        m_num_tasks(m_num_tiles_per_dim*m_num_tiles_per_dim),
        m_tasks(new sampling_task[m_num_tasks]),
        m_task_storages(new sampling_task_data[m_num_tasks]),
        m_fact(fact),
        m_rng(100),
        m_samps(0)
{
        m_thrpool = new e8util::thread_pool(m_num_tasks);

        // create task constructs.
        for (unsigned i = 0; i < m_num_tasks; i ++) {
                m_tasks[i] = sampling_task(m_fact->create());
        }
}

e8::pt_image_renderer::~pt_image_renderer()
{
        delete [] m_tasks;
        delete [] m_task_storages;
        delete m_thrpool;
        delete m_fact;
}

void
e8::pt_image_renderer::render(if_path_space const* scene, if_camera const* cam, if_compositor* compositor)
{
        // generate camera seed ray, if the update is dirty, for each tile task.
        if (update_image_view(cam, compositor)) {
                for (unsigned j = 0; j < m_num_tiles_per_dim; j ++) {
                        for (unsigned i = 0; i < m_num_tiles_per_dim; i ++) {
                                unsigned tile_w = i == m_num_tiles_per_dim - 1 ? m_w - m_w/m_num_tiles_per_dim*i : m_w/m_num_tiles_per_dim;
                                unsigned tile_h = j == m_num_tiles_per_dim - 1 ? m_h - m_h/m_num_tiles_per_dim*j : m_h/m_num_tiles_per_dim;
                                m_task_storages[i + j*m_num_tiles_per_dim].rays.resize(tile_w*tile_h);
                                m_task_storages[i + j*m_num_tiles_per_dim].scene = scene;
                                m_task_storages[i + j*m_num_tiles_per_dim].set_data_id(static_cast<e8util::data_id_t>(i + j*m_num_tiles_per_dim));

                                unsigned top_left_i = m_w/m_num_tiles_per_dim*i;
                                unsigned top_left_j = m_h/m_num_tiles_per_dim*j;
                                std::vector<e8util::ray>& tile_rays = m_task_storages[i + j*m_num_tiles_per_dim].rays;
                                for (unsigned tj = 0; tj < tile_h; tj ++) {
                                        for (unsigned ti = 0; ti < tile_w; ti ++) {
                                                float pdf;
                                                tile_rays[ti + tj*tile_w] = cam->sample(m_rng,
                                                                                        top_left_i + ti, top_left_j + tj,
                                                                                        m_w, m_h, pdf);
                                        }
                                }
                        }
                }

                m_rad.resize(m_w*m_h);
                for (unsigned i = 0; i < m_w*m_h; i ++)
                        m_rad[i] = 0.0f;
                m_samps = 0;
        }

        // launch tasks.
        for (unsigned i = 0; i < m_num_tasks; i ++) {
                m_thrpool->run(&m_tasks[i], &m_task_storages[i]);
        }

        // retrieve and accumulate estimated values.
        m_samps += 1;
        float pr = 1.0f/m_samps;
        for (unsigned k = 0; k < m_num_tasks; k ++) {
                e8util::task_info task_info = m_thrpool->retrieve_next_completed();
                unsigned i = static_cast<unsigned>(task_info.task_storage()->data_id())%m_num_tiles_per_dim;
                unsigned j = static_cast<unsigned>(task_info.task_storage()->data_id())/m_num_tiles_per_dim;

                unsigned tile_w = i == m_num_tiles_per_dim - 1 ? m_w - m_w/m_num_tiles_per_dim*i : m_w/m_num_tiles_per_dim;
                unsigned tile_h = j == m_num_tiles_per_dim - 1 ? m_h - m_h/m_num_tiles_per_dim*j : m_h/m_num_tiles_per_dim;

                unsigned top_left_i = m_w/m_num_tiles_per_dim*i;
                unsigned top_left_j = m_h/m_num_tiles_per_dim*j;
                unsigned task_i = i + j*m_num_tiles_per_dim;

                std::vector<e8util::vec3> const& estimates = m_tasks[task_i].get_estimates();
                for (unsigned tj = 0; tj < tile_h; tj ++) {
                        for (unsigned ti = 0; ti < tile_w; ti ++) {
                                unsigned p = (top_left_i + ti) + (top_left_j + tj)*m_w;
                                m_rad[p] = m_rad[p] + estimates[ti + tj*tile_w];
                                e8util::vec3 const& r = pr*m_rad[p];
                                (*compositor)(top_left_i + ti, top_left_j + tj) = r.homo(1.0f);
                        }
                }
        }
}

e8::rendering_stats
e8::pt_image_renderer::get_stats() const
{
        throw std::string("Not implemented yet");
}
