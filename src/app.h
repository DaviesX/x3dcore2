#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QTimer>
#include "resource.h"
#include "scene.h"
#include "compositor.h"
#include "frame.h"
#include "camera.h"
#include "renderer.h"
#include "thread.h"

namespace Ui {
class MainWindow;
}


class rendering_task: public e8util::if_task
{
public:
        rendering_task();
        ~rendering_task();

        void    main(void* storage) override;
        void    update();
        bool    is_running() const;
        void    enable(bool state);

        struct settings {
                std::string     scene;
                std::string     renderer;
                std::string     layout;
                float           exposure;
                unsigned        num_samps;


                bool operator==(settings const& rhs) const
                {
                        return scene == rhs.scene &&
                                renderer == rhs.renderer &&
                                        layout == rhs.layout &&
                                        exposure == rhs.exposure &&
                                        num_samps == rhs.num_samps;
                }

                bool operator!=(settings const& rhs) const
                {
                        return !((*this) == rhs);
                }
        };

        e8::if_scene*           m_scene = nullptr;
        e8::if_im_renderer*     m_renderer = nullptr;
        e8::aces_compositor*    m_com = nullptr;
        e8::if_frame*           m_frame = nullptr;
        e8::if_camera*          m_cam = nullptr;

        settings                m_old;
        settings                m_current;
        e8util::mutex_t         m_mutex;
        bool                    m_is_running = false;
};

class App : public QMainWindow
{
        Q_OBJECT

public:
        explicit App(QWidget *parent = 0);
        ~App();

private slots:
        void on_check_autoexposure_stateChanged(int arg1);

        void on_button_save_clicked();

        void on_button_render_clicked();

        void on_MainWindow_destroyed();

        void on_action_openfile_triggered();

        void on_actionCornellball_triggered();

        void on_update_stats();
private:
        Ui::MainWindow*         m_ui;

        QTimer                  m_stats_update_timer;

        e8::ram_ogl_frame*      m_frame;

        rendering_task          m_task;
        e8util::task_info       m_info;
};

#endif // APP_H
