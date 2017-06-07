#include <iostream>
#include <QFileDialog>
#include "app.h"
#include "ui_mainwindow.h"


rendering_task::rendering_task()
{
        m_mutex = e8util::mutex();
}

rendering_task::~rendering_task()
{
        e8util::destroy(m_mutex);
}

void
rendering_task::main(void*)
{
        while (m_is_running) {
                e8util::lock(m_mutex);
                e8util::unlock(m_mutex);
        }
}

void
rendering_task::update()
{
        e8util::lock(m_mutex);
        e8util::unlock(m_mutex);
}

bool
rendering_task::is_running() const
{
        return m_is_running;
}

void
rendering_task::enable(bool state)
{
        m_is_running = state;
}



App::App(QWidget *parent) :
        QMainWindow(parent),
        m_ui(new Ui::MainWindow)
{
        m_ui->setupUi(this);

        m_ui->combo_tracer->addItem("direct lighting");
        m_ui->combo_tracer->addItem("unidirectional tracing");
        m_ui->combo_tracer->addItem("bidirectional tracing");

        m_ui->combo_structure->addItem("linear");
        m_ui->combo_structure->addItem("static bvh");

        connect(&m_timer, SIGNAL(timeout()), this, SLOT(on_update_stats()));

        m_ui->statusbar->showMessage("e8yescg started.");
}

App::~App()
{
        delete m_ui;
}

void
App::on_check_autoexposure_stateChanged(int)
{
        if (m_ui->check_autoexposure->isChecked())
                m_ui->spin_manualexposure->setEnabled(false);
        else
                m_ui->spin_manualexposure->setEnabled(true);
}

void
App::on_button_save_clicked()
{
        if (m_task.is_running()) {
                m_ui->statusbar->showMessage("Cannot save rendering result while the renderer is running. Please pause the renderer first.");
                return;
        }
        QString file_name = QFileDialog::getSaveFileName(this,
            tr("Save Image"), ".", tr("Image Files (*.png)"));
        if (file_name.length() > 0) {
                QImage const& img = m_ui->ogl_surface->grabFramebuffer();
                if (img.save(file_name, "png"))
                        m_ui->statusbar->showMessage("Image has been saved to " + file_name + ".");
                else
                        m_ui->statusbar->showMessage("Failed to save image to " + file_name + ".");
        }
}

void
App::on_button_render_clicked()
{
        if (m_task.is_running()) {
                m_task.enable(false);
                e8util::sync(m_info);

                m_ui->button_render->setText("start");
                m_ui->statusbar->showMessage("paused.");
                m_timer.stop();
        } else {
                m_task.enable(true);
                m_info = e8util::run(&m_task);

                m_timer.start(500);
                m_ui->button_render->setText("pause");
                m_ui->statusbar->showMessage("rendering...");
        }
}

void
App::on_update_stats()
{
        std::cout << "Updating stats" << std::endl;
}

void
App::on_MainWindow_destroyed()
{
        m_task.enable(false);
        e8util::sync(m_info);
}

void App::on_action_openfile_triggered()
{
        QString file_name = QFileDialog::getOpenFileName(this,
            tr("Open scene"), ".", tr("Scene File (*.e8yescg)"));
        if (file_name.length() > 0) {
                m_ui->statusbar->showMessage("Using scene " + file_name + ".");
        }
}

void App::on_actionCornellball_triggered()
{
        m_ui->statusbar->showMessage("Using scene Cornellball.");
}
