#ifndef PIPELINE_H
#define PIPELINE_H

#include <ctime>
#include "resource.h"
#include "pathspace.h"
#include "compositor.h"
#include "frame.h"
#include "camera.h"
#include "renderer.h"
#include "thread.h"
#include "objdb.h"

namespace e8
{

class if_render_pipeline: public e8util::if_task
{
public:
        if_render_pipeline();
        virtual ~if_render_pipeline() override;

        void            run(e8util::if_task_storage* unused = nullptr) override;
        void            push_updates();

        e8::objdb&      objdb();

        bool            is_running() const;
        void            enable();
        void            disable();

        unsigned        frame_no() const;
        float           time_elapsed() const;
protected:
        virtual void    render_frame() = 0;
        virtual void    update_pipeline() = 0;

        e8::objdb               m_objdb;
        unsigned                m_frame_no = 0;
        bool                    m_is_running = false;

private:
        std::clock_t            m_task_started = 0;
        e8util::mutex_t         m_mutex;
};

class pt_render_pipeline: public if_render_pipeline
{
public:
        pt_render_pipeline();
        virtual ~pt_render_pipeline() override;

        void            render_frame() override;
        void            update_pipeline() override;

        struct settings
        {
                settings();

                bool    operator==(settings const& rhs) const;
                bool    operator!=(settings const& rhs) const;

                std::string     scene;
                std::string     renderer;
                std::string     layout;
                float           exposure;
                unsigned        num_samps;
        };

        e8::if_path_space*      m_scene = nullptr;
        e8::pt_image_renderer*  m_renderer = nullptr;
        e8::aces_compositor*    m_com = nullptr;
        e8::if_frame*           m_frame = nullptr;
        e8::if_camera*          m_cam = nullptr;

        settings                m_old;
        settings                m_current;

};

}

#endif // PIPELINE_H
