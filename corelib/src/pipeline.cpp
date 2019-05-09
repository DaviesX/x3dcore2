#include <cassert>
#include "pipeline.h"


e8::pt_rendering_pipeline::settings::settings():
        scene("cornellball"),
        renderer("normal"),
        layout("linear"),
        exposure(1.0f),
        num_samps(1)
{
}

bool
e8::pt_rendering_pipeline::settings::operator==(settings const& rhs) const
{
        return scene == rhs.scene &&
                renderer == rhs.renderer &&
                        layout == rhs.layout &&
                        exposure == rhs.exposure &&
                        num_samps == rhs.num_samps;
}

bool
e8::pt_rendering_pipeline::settings::operator!=(settings const& rhs) const
{
        return !((*this) == rhs);
}



e8::pt_rendering_pipeline::pt_rendering_pipeline()
{
        m_mutex = e8util::mutex();
        m_com = new e8::aces_compositor(0, 0);
        update();
}

e8::pt_rendering_pipeline::~pt_rendering_pipeline()
{
        e8util::destroy(m_mutex);
}

void
e8::pt_rendering_pipeline::run(e8util::if_task_storage*)
{
        while (m_is_running) {
                e8util::lock(m_mutex);
                m_com->resize(m_frame->width(), m_frame->height());
                m_com->enable_auto_exposure(false);
                m_com->exposure(m_old.exposure);
                m_renderer->render(m_scene, m_cam, m_com);
                m_com->commit(m_frame);
                m_frame->commit();
                m_num_commits ++;
                e8util::unlock(m_mutex);
        }
}

void
e8::pt_rendering_pipeline::update()
{
        e8util::lock(m_mutex);
        if (m_current != m_old) {
                // update.
                if (m_renderer == nullptr || m_current.renderer != m_old.renderer) {
                        delete m_renderer;

                        e8::pathtracer_factory::pt_type pt_type = e8::pathtracer_factory::pt_type::normal;
                        if (m_current.renderer == "normal") {
                                pt_type = e8::pathtracer_factory::pt_type::normal;
                        } else if (m_current.renderer == "position") {
                                pt_type = e8::pathtracer_factory::pt_type::position;
                        } else if (m_current.renderer == "direct") {
                                pt_type = e8::pathtracer_factory::pt_type::direct;
                        } else if (m_current.renderer == "unidirectional") {
                                pt_type = e8::pathtracer_factory::pt_type::unidirect;
                        } else if (m_current.renderer == "bidirectional (LT2)") {
                                pt_type = e8::pathtracer_factory::pt_type::bidirect_lt2;
                        } else if (m_current.renderer == "bidirectional (MIS)") {
                                pt_type = e8::pathtracer_factory::pt_type::bidirect_mis;
                        }
                        m_renderer = new e8::pt_image_renderer(new e8::pathtracer_factory(pt_type,
                                                                                          e8::pathtracer_factory::options()));
                }
                bool scene_layout_changed = false;
                if (m_scene == nullptr || m_current.layout != m_old.layout) {
                        scene_layout_changed = true;
                        delete m_scene;
                        if (m_current.layout == "linear") {
                                m_scene = new e8::linear_path_space_layout();
                        } else if (m_current.layout == "static bvh") {
                                m_scene = new e8::bvh_path_space_layout();
                        } else {
                                assert(m_current.layout == "linear" ||
                                       m_current.layout == "static bvh");
                        }
                }
                if (scene_layout_changed || m_current.scene != m_old.scene) {
                        if (m_current.scene == "cornellball") {
                                e8util::cornell_scene res;
                                m_scene->load(&res);
                                m_scene->commit();
                                m_cam = res.load_camera();
                        } else {
                                e8util::gltf_scene res(m_current.scene);
                                m_scene->load(&res);
                                m_scene->commit();
                                m_cam = res.load_camera();
                        }
                }
                m_old = m_current;
                m_num_commits = 0;
                m_task_started = std::clock();
        }
        e8util::unlock(m_mutex);
}

bool
e8::pt_rendering_pipeline::is_running() const
{
        return m_is_running;
}

void
e8::pt_rendering_pipeline::enable(bool state)
{
        m_is_running = state;
}

uint32_t
e8::pt_rendering_pipeline::num_commits() const
{
        return m_num_commits;
}

float
e8::pt_rendering_pipeline::time_elapsed() const
{
        return static_cast<float>(std::clock() - m_task_started)/CLOCKS_PER_SEC;
}
