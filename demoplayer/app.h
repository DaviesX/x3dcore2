#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QTimer>
#include <ctime>
#include "../corelib/src/pipeline.h"

namespace Ui
{
class MainWindow;
}


class App : public QMainWindow
{
        Q_OBJECT

public:
        explicit App(QWidget *parent = nullptr);
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
        Ui::MainWindow*                 m_ui;

        QTimer                          m_stats_update_timer;

        e8::ram_ogl_frame*              m_frame;
        e8::if_render_pipeline*         m_pipeline;
        e8util::task_info               m_pipeline_task;
        e8util::flex_config             m_pipeline_config;
};

#endif // APP_H
