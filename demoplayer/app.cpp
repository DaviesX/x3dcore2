#include <iostream>
#include <string>
#include <QFileDialog>
#include "app.h"
#include "ui_mainwindow.h"



App::App(QWidget *parent) :
        QMainWindow(parent),
        m_ui(new Ui::MainWindow)
{
        m_ui->setupUi(this);

        m_ui->combo_tracer->addItem("direct tracing");
        m_ui->combo_tracer->addItem("unidirectional tracing");
        m_ui->combo_tracer->addItem("bidirectional tracing");
        m_ui->combo_tracer->addItem("bidirectional tracing (MIS)");
        m_ui->combo_tracer->addItem("position tracing");
        m_ui->combo_tracer->addItem("normal tracing");

        m_ui->combo_structure->addItem("static bvh");
        m_ui->combo_structure->addItem("linear");


        connect(&m_stats_update_timer, SIGNAL(timeout()), this, SLOT(on_update_stats()));

        m_frame = new e8::ram_ogl_frame(m_ui->centralwidget);
        m_ui->gridLayout->addWidget(m_frame, 0, 0, 10, 1);

        m_ui->statusbar->showMessage("e8yescg started.");
}

App::~App()
{
        delete m_ui;
        delete m_frame;
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
                QImage const& img = m_frame->grabFramebuffer();
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
                m_stats_update_timer.stop();
        } else {
                m_task.m_frame = m_frame;
                m_task.m_current.exposure = m_ui->spin_manualexposure->value();
                m_task.m_current.layout = m_ui->combo_structure->currentText().toStdString();
                m_task.m_current.renderer = m_ui->combo_tracer->currentText().toStdString();
                m_task.m_current.num_samps = m_ui->spin_sample->value();
                m_task.m_current.scene = "cornellball";
                m_task.update();

                m_task.enable(true);
                m_info = e8util::run(&m_task);

                m_stats_update_timer.start(500);
                m_ui->button_render->setText("pause");
                m_ui->statusbar->showMessage("rendering...");
        }
}

void
App::on_update_stats()
{
        //std::cout << "Updating stats" << std::endl;
        m_frame->repaint();
        m_ui->label_time->setText(QString::fromStdString(std::to_string(m_task.time_elapsed()) + " s"));
        m_ui->label_samp_count1->setText(QString::fromStdString(std::to_string(m_task.num_commits())));
        m_ui->label_samp_count2->setText(QString::fromStdString(std::to_string(m_task.num_commits())));
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
