#ifndef APP_H
#define APP_H

#include <QMainWindow>
#include <QTimer>
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
private:
        struct settings {
        };

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

        QTimer                  m_timer;

        rendering_task          m_task;
        e8util::task_info       m_info;
};

#endif // APP_H
