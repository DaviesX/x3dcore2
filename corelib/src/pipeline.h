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

namespace e8
{

class pt_rendering_pipeline: public e8util::if_task
{
public:
        pt_rendering_pipeline();
        virtual ~pt_rendering_pipeline() override;

        void            run(e8util::if_task_storage* storage) override;
        void            update();
        bool            is_running() const;
        void            enable(bool state);
        uint32_t        num_commits() const;
        float           time_elapsed() const;

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

        e8::if_path_space*           m_scene = nullptr;
        e8::if_im_renderer*     m_renderer = nullptr;
        e8::aces_compositor*    m_com = nullptr;
        e8::if_frame*           m_frame = nullptr;
        e8::if_camera*          m_cam = nullptr;

        settings                m_old;
        settings                m_current;
        e8util::mutex_t         m_mutex;
        bool                    m_is_running = false;
        std::clock_t            m_task_started = 0;
        uint32_t                m_num_commits = 0;
};

}

#endif // PIPELINE_H
