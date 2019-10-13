#include <cmath>

#include "renderer.h"
#include "tensor.h"
#include "thread.h"

e8::pt_image_renderer::sampling_task_data::sampling_task_data(
    e8util::data_id_t id, if_path_space const &path_space, if_light_sources const &light_sources,
    std::vector<e8util::ray> const &rays, if_pathtracer::first_hits const &first_hits,
    unsigned num_samps, unsigned width, unsigned height, bool firefly_filter)
    : e8util::if_task_storage(id), path_space(path_space), light_sources(light_sources), rays(rays),
      first_hits(first_hits), num_samps(num_samps), width(width), height(height),
      firefly_filter(firefly_filter) {}

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

    // Allocate and clear result buffer.
    m_estimate.resize(data->rays.size());
    std::fill(m_estimate.begin(), m_estimate.end(), e8util::vec3());

    // Compute and accumulate multi-sample estimate.
    for (unsigned i = 0; i < data->num_samps; i++) {
        std::vector<e8util::vec3> estimate = m_pt->sample(m_rng, data->rays, data->first_hits,
                                                          data->path_space, data->light_sources);
        if (data->firefly_filter) {
            for (unsigned y = 0; y < data->height; y++) {
                for (unsigned x = 0; x < data->width; x++) {
                    if (x > 0 && x < data->width - 1 && y > 0 && y < data->height - 1) {
                        // Check if all 8 neighboring pixels are all within the firefly threshold,
                        // otherwise cap the difference.
                        float r00 = estimate[x - 1 + (y - 1) * data->width].norm2();
                        float r10 = estimate[x + (y - 1) * data->width].norm2();
                        float r20 = estimate[x + 1 + (y - 1) * data->width].norm2();

                        float r01 = estimate[x - 1 + y * data->width].norm2();
                        float r11 = estimate[x + y * data->width].norm2();
                        float r21 = estimate[x + 1 + y * data->width].norm2();

                        float r02 = estimate[x - 1 + (y + 1) * data->width].norm2();
                        float r12 = estimate[x + (y + 1) * data->width].norm2();
                        float r22 = estimate[x + 1 + (y + 1) * data->width].norm2();

                        bool firefly =
                            (r11 - r00) > FireFlyMinDiff && (r11 - r10) > FireFlyMinDiff &&
                            (r11 - r20) > FireFlyMinDiff && (r11 - r01) > FireFlyMinDiff &&
                            (r11 - r21) > FireFlyMinDiff && (r11 - r02) > FireFlyMinDiff &&
                            (r11 - r12) > FireFlyMinDiff && (r11 - r22) > FireFlyMinDiff;
                        if (firefly) {
                            float cap =
                                1.0f / 8.0f * (r00 + r10 + r20 + r01 + r21 + r02 + r12 + r22) +
                                FireFlyMinDiff;
                            m_estimate[x + y * data->width] +=
                                estimate[x + y * data->width].at_most(cap);
                        } else {
                            m_estimate[x + y * data->width] += estimate[x + y * data->width];
                        }
                    } else {
                        // Just copy the egde of the image because there is not enough sample.
                        m_estimate[x + y * data->width] += estimate[x + y * data->width];
                    }
                }
            }
        } else {
            for (unsigned j = 0; j < estimate.size(); j++) {
                m_estimate[j] += estimate[j];
            }
        }
    }

    if (data->num_samps > 1) {
        // Average estimate if there are more than 1 sample.
        float scale = 1.0f / data->num_samps;
        for (unsigned i = 0; i < m_estimate.size(); i++) {
            m_estimate[i] *= scale;
        }
    }
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
e8::pt_image_renderer::render(if_compositor *compositor, if_path_space const &path_space,
                              if_light_sources const &light_sources, if_camera const &cam,
                              unsigned num_samps, bool firefly_filter) {
    // Generate camera seed rays and first_hits
    std::vector<e8util::ray> rays(compositor->width() * compositor->height());
    for (unsigned j = 0; j < compositor->height(); j++) {
        for (unsigned i = 0; i < compositor->width(); i++) {
            float pdf;
            rays[i + j * compositor->width()] =
                cam.sample(m_rng, i, j, compositor->width(), compositor->height(), pdf);
        }
    }

    if_pathtracer::first_hits first_hits = if_pathtracer::compute_first_hit(rays, path_space);

    // Launch tasks.
    unsigned allocated_samps =
        static_cast<unsigned>(std::ceil(static_cast<float>(num_samps) / m_tasks.size()));
    sampling_task_data task_config(/*id=*/0, path_space, light_sources, rays, first_hits,
                                   allocated_samps, compositor->width(), compositor->height(),
                                   firefly_filter);
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
