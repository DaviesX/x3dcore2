#ifndef PIPELINE_H
#define PIPELINE_H

#include "objdb.h"
#include "thread.h"
#include "util.h"
#include <ctime>
#include <memory>

namespace e8 {
class aces_compositor;
}
namespace e8 {
class if_frame;
}
namespace e8 {
class pt_image_renderer;
}

namespace e8 {

/**
 * @brief The if_render_pipeline class General rendering pipeline interface.
 */
class if_render_pipeline : public e8util::if_task {
  public:
    if_render_pipeline(if_frame *target);
    virtual ~if_render_pipeline() override;

    void config(e8util::flex_config const &new_config);
    e8util::flex_config config() const;
    void run(e8util::if_task_storage *unused = nullptr) override;

    e8::objdb &objdb();

    bool is_running() const;
    void enable();
    void disable();

    unsigned frame_no() const;
    float time_elapsed() const;

  protected:
    virtual void render_frame() = 0;
    virtual void update_pipeline(e8util::flex_config const &diff) = 0;
    virtual e8util::flex_config config_protocol() const = 0;

    e8::objdb m_objdb;
    e8::if_frame *m_frame;
    unsigned m_frame_no = 0;
    bool m_is_running = false;

  private:
    std::clock_t m_task_started = 0;
    e8util::mutex_t m_mutex;
    e8util::flex_config m_old_config;
};

/**
 * @brief The pt_render_pipeline class High-level encapsulation of the path-tracing rendering
 * pipeline.
 */
class pt_render_pipeline : public if_render_pipeline {
  public:
    pt_render_pipeline(if_frame *target);
    virtual ~pt_render_pipeline() override;

    void render_frame() override;
    void update_pipeline(e8util::flex_config const &diff) override;
    e8util::flex_config config_protocol() const override;

  private:
    std::unique_ptr<e8::pt_image_renderer> m_renderer;
    std::unique_ptr<e8::aces_compositor> m_com;
    unsigned m_num_threads = 0;
    unsigned m_samps_per_frame = 1;
    bool m_firefly_filter = true;
};

} // namespace e8

#endif // PIPELINE_H
