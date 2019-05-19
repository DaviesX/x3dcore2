#include <cassert>
#include "pipeline.h"


e8::if_render_pipeline::if_render_pipeline(if_frame* target):
        m_frame(target),
        m_mutex(e8util::mutex())
{
        assert(m_frame != nullptr);
}

e8::if_render_pipeline::~if_render_pipeline()
{
        e8util::destroy(m_mutex);
}

void
e8::if_render_pipeline::run(e8util::if_task_storage* /* unused */)
{
        m_task_started = std::clock();
        while (m_is_running) {
                e8util::lock(m_mutex);

                render_frame();
                m_frame_no ++;

                e8util::unlock(m_mutex);
        }
}

void
e8::if_render_pipeline::config(e8util::flex_config const& new_config)
{
        e8util::lock(m_mutex);

        update_pipeline(new_config - m_old_config);
        m_old_config = new_config;

        e8util::unlock(m_mutex);
}

e8util::flex_config
e8::if_render_pipeline::config() const
{
        return m_old_config;
}

e8::objdb&
e8::if_render_pipeline::objdb()
{
        return m_objdb;
}

bool
e8::if_render_pipeline::is_running() const
{
        return m_is_running;
}

void
e8::if_render_pipeline::enable()
{
        m_is_running = true;
}

void
e8::if_render_pipeline::disable()
{
        m_is_running = false;
}

unsigned
e8::if_render_pipeline::frame_no() const
{
        return m_frame_no;
}

float
e8::if_render_pipeline::time_elapsed() const
{
        return static_cast<float>(std::clock() - m_task_started)/CLOCKS_PER_SEC;
}


e8::pt_render_pipeline::pt_render_pipeline(if_frame* target):
        if_render_pipeline(target)
{
        m_com = new e8::aces_compositor(0, 0);
        m_cinematics = new e8::stationary_cam_controller("default_cinematics");

        update_pipeline(config_protocol());

        m_objdb.register_manager(m_path_space);
        m_objdb.register_manager(m_cinematics);
}

e8::pt_render_pipeline::~pt_render_pipeline()
{
}

void
e8::pt_render_pipeline::render_frame()
{
        m_com->resize(m_frame->width(), m_frame->height());
        m_objdb.push_updates();
        if_camera* cur_cam = m_cinematics->main_cam();
        if (cur_cam != nullptr) {
                m_renderer->render(m_path_space, cur_cam, m_com);
        }
        m_com->commit(m_frame);
        m_frame->commit();
}

e8util::flex_config
e8::pt_render_pipeline::config_protocol() const
{
        e8util::flex_config config;
        config.str_val["scene_file"] = "cornellball";
        config.str_val["path_space"] = "static_bvh";
        config.enum_vals["path_space"] = std::set<std::string> {
                        "linear",
                        "static_bvh"};
        config.enum_sel["path_space"] = "static_bvh";
        config.enum_vals["path_tracer"] = std::set<std::string> {
                        "normal",
                        "position",
                        "direct",
                        "unidirectional",
                        "bidirectional_lt2",
                        "bidirectional_mis"};
        config.enum_sel["path_tracer"] = "unidirectional";
        config.bool_val["auto_exposure"] = false;
        config.float_val["exposure"] = 1.0f;
        config.int_val["super_samples"] = 16;
        return config;
}

void
e8::pt_render_pipeline::update_pipeline(e8util::flex_config const& diff)
{
        // update.
        diff.find_enum("path_tracer", [this] (std::string const& tracer_type,
                                              e8util::flex_config const* /*config*/) {
                delete m_renderer;
                e8::pathtracer_factory::pt_type pt_type = e8::pathtracer_factory::pt_type::normal;
                if (tracer_type == "normal") {
                        pt_type = e8::pathtracer_factory::pt_type::normal;
                } else if (tracer_type == "position") {
                        pt_type = e8::pathtracer_factory::pt_type::position;
                } else if (tracer_type == "direct") {
                        pt_type = e8::pathtracer_factory::pt_type::direct;
                } else if (tracer_type == "unidirectional") {
                        pt_type = e8::pathtracer_factory::pt_type::unidirect;
                } else if (tracer_type == "bidirectional_lt2") {
                        pt_type = e8::pathtracer_factory::pt_type::bidirect_lt2;
                } else if (tracer_type == "bidirectional_mis") {
                        pt_type = e8::pathtracer_factory::pt_type::bidirect_mis;
                }
                m_renderer = new e8::pt_image_renderer(new e8::pathtracer_factory(pt_type,
                                                                                  e8::pathtracer_factory::options()));
        });

        // @FIXME: mark objects in the obj manager dirty because the new obj manager is empty.
        diff.find_enum("path_space", [this] (std::string const& path_space_type,
                                             e8util::flex_config const* /*config*/) {
                delete m_path_space;
                if (path_space_type == "linear") {
                        m_path_space = new e8::linear_path_space_layout();
                } else if (path_space_type == "static_bvh") {
                        m_path_space = new e8::bvh_path_space_layout();
                }
        });

        diff.find_str("scene_file", [this] (std::string const& scene_file) {
                m_objdb.clear();
                if (scene_file == "cornellball") {
                        m_objdb.manage_roots(e8util::cornell_scene().load_roots());
                } else {
                        m_objdb.manage_roots(e8util::gltf_scene(scene_file).load_roots());
                }
        });

        diff.find_bool("auto_exposure", [this] (bool const& val) {
                m_com->enable_auto_exposure(val);
        });
        diff.find_float("exposure", [this] (float const& val) {
                m_com->exposure(val);
        });
}
