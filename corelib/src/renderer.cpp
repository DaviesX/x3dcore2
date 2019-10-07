#include <cmath>

#include "renderer.h"
#include "tensor.h"
#include "thread.h"

e8::pt_image_renderer::sampling_task_data::sampling_task_data(e8util::data_id_t id,
                                                              if_path_space const &path_space,
                                                              if_light_sources const &light_sources,
                                                              std::vector<e8util::ray> const &rays,
                                                              unsigned num_samps)
    : e8util::if_task_storage(id), path_space(path_space), light_sources(light_sources), rays(rays),
      num_samps(num_samps) {}

e8::pt_image_renderer::sampling_task::sampling_task() : e8util::if_task(false), m_pt(nullptr) {}

e8::pt_image_renderer::sampling_task::sampling_task(e8::if_pathtracer *pt, unsigned seed)
    : e8util::if_task(false), m_rng(seed), m_pt(pt) {}

e8::pt_image_renderer::sampling_task::sampling_task(sampling_task &&rhs) {
    m_estimate = rhs.m_estimate;
    m_rng = rhs.m_rng;
    m_pt = rhs.m_pt;
    rhs.m_pt = nullptr;
}

e8::pt_image_renderer::sampling_task::~sampling_task() { delete m_pt; }

e8::pt_image_renderer::sampling_task &e8::pt_image_renderer::sampling_task::
operator=(sampling_task rhs) {
    m_estimate = rhs.m_estimate;
    m_rng = rhs.m_rng;
    std::swap(m_pt, rhs.m_pt);
    return *this;
}

void e8::pt_image_renderer::sampling_task::run(e8util::if_task_storage *p) {
    sampling_task_data *data = static_cast<sampling_task_data *>(p);
    m_estimate =
        m_pt->sample(m_rng, data->rays, data->path_space, data->light_sources, data->num_samps);
}

std::vector<e8util::vec3> const &e8::pt_image_renderer::sampling_task::get_estimates() const {
    return m_estimate;
}

e8::pt_image_renderer::pt_image_renderer(std::unique_ptr<pathtracer_factory> fact)
    : m_tasks(e8util::cpu_core_count()), m_thrpool(e8util::cpu_core_count()), m_rng(1361) {
    // create task constructs.
    for (unsigned i = 0; i < m_tasks.size(); i++) {
        m_tasks[i] = sampling_task(fact->create(), i * 1361 + 33);
    }
}

e8::pt_image_renderer::numerical_stats
e8::pt_image_renderer::render(if_path_space const &path_space,
                              if_light_sources const &light_sources, if_camera const &cam,
                              unsigned num_samps, if_compositor *compositor) {
    // Generate camera seed rays
    std::vector<e8util::ray> rays(compositor->width() * compositor->height());
    for (unsigned j = 0; j < compositor->height(); j++) {
        for (unsigned i = 0; i < compositor->width(); i++) {
            float pdf;
            rays[i + j * compositor->width()] =
                cam.sample(m_rng, i, j, compositor->width(), compositor->height(), pdf);
        }
    }

    // Launch tasks.
    unsigned allocated_samps =
        static_cast<unsigned>(std::ceil(static_cast<float>(num_samps) / m_tasks.size()));
    sampling_task_data task_config(/*id=*/0, path_space, light_sources, rays, allocated_samps);
    for (unsigned i = 0; i < m_tasks.size(); i++) {
        m_thrpool.run(&m_tasks[i], &task_config);
    }

    for (unsigned j = 0; j < compositor->height(); j++) {
        for (unsigned i = 0; i < compositor->width(); i++) {
            (*compositor)(i, j) = 0.0f;
        }
    }

    // Retrieve and accumulate estimated values.
    for (unsigned i = 0; i < m_tasks.size(); i++) {
        e8util::task_info result = m_thrpool.retrieve_next_completed();
        std::vector<e8util::vec3> const &estimates =
            static_cast<sampling_task *>(result.task())->get_estimates();

        for (unsigned j = 0; j < compositor->height(); j++) {
            for (unsigned i = 0; i < compositor->width(); i++) {
                (*compositor)(i, j) += estimates[i + j * compositor->width()].homo(1.0f);
            }
        }
    }

    // Average estimates.
    float scale = 1.0f / m_tasks.size();
    for (unsigned j = 0; j < compositor->height(); j++) {
        for (unsigned i = 0; i < compositor->width(); i++) {
            (*compositor)(i, j) *= scale;
        }
    }

    // TODO: Complete all stats.
    numerical_stats stats;
    stats.num_samples = allocated_samps * static_cast<unsigned>(m_tasks.size());

    return stats;
}
