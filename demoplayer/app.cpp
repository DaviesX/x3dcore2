#include <memory>

#include "app.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <iostream>
#include <string>

App::App(QWidget *parent) : QMainWindow(parent), m_ui(std::make_unique<Ui::MainWindow>()) {
    m_ui->setupUi(this);

    m_ui->combo_renderer->addItem("MCPT renderer");

    m_ui->combo_tracer->addItem("bidirectional (MIS)");
    m_ui->combo_tracer->addItem("bidirectional");
    m_ui->combo_tracer->addItem("unidirectional");
    m_ui->combo_tracer->addItem("direct");
    m_ui->combo_tracer->addItem("normal");
    m_ui->combo_tracer->addItem("position");

    m_ui->combo_structure->addItem("static bvh");
    m_ui->combo_structure->addItem("linear");

    connect(&m_stats_update_timer, SIGNAL(timeout()), this, SLOT(on_update_stats()));

    m_frame = std::make_unique<e8::ram_ogl_frame>(m_ui->centralwidget);
    m_pipeline = std::make_unique<e8::pt_render_pipeline>(m_frame.get());
    m_pipeline_config = m_pipeline->config();
    m_ui->gridLayout->addWidget(m_frame.get(), 0, 0, 10, 1);

    m_ui->statusbar->showMessage("e8yescg started.");
}

App::~App() {}

void App::on_check_autoexposure_stateChanged(int) {
    if (m_ui->check_autoexposure->isChecked())
        m_ui->spin_manualexposure->setEnabled(false);
    else
        m_ui->spin_manualexposure->setEnabled(true);
}

void App::on_button_save_clicked() {
    if (m_pipeline->is_running()) {
        m_ui->statusbar->showMessage("Cannot save rendering result while the render pipeline"
                                     " is still running. Please pause the pipeline.");
        return;
    }
    QString file_name =
        QFileDialog::getSaveFileName(this, tr("Save Image"), ".", tr("Image Files (*.png)"));
    if (file_name.length() > 0) {
        QImage const &img = m_frame->grabFramebuffer();
        if (img.save(file_name, "png"))
            m_ui->statusbar->showMessage("Image has been saved to " + file_name + ".");
        else
            m_ui->statusbar->showMessage("Failed to save image to " + file_name + ".");
    }
}

void App::on_button_render_clicked() {
    if (m_pipeline->is_running()) {
        // Stop the render pipeline.
        m_pipeline->disable();
        e8util::sync(m_pipeline_task);

        m_ui->button_render->setText("start");
        m_ui->statusbar->showMessage("paused.");
        m_stats_update_timer.stop();
    } else {
        // Start the render pipeline.
        m_pipeline->config(m_pipeline_config);
        m_pipeline->enable();
        m_pipeline_task = e8util::run(m_pipeline.get());

        m_stats_update_timer.start(500);
        m_ui->button_render->setText("pause");
        m_ui->statusbar->showMessage("rendering...");
    }
}

void App::on_update_stats() {
    // std::cout << "Updating stats" << std::endl;
    m_frame->repaint();
    m_ui->label_time->setText(
        QString::fromStdString(std::to_string(m_pipeline->time_elapsed()) + " s"));
    m_ui->label_samp_count1->setText(
        QString::fromStdString(std::to_string(m_pipeline->frame_no())));
}

void App::on_MainWindow_destroyed() {
    m_pipeline->disable();
    e8util::sync(m_pipeline_task);
}

void App::on_action_openfile_triggered() {
    if (m_pipeline->is_running()) {
        m_ui->statusbar->showMessage(
            "Render pipeline is running. Need to stop the rendering task first.");
        return;
    } else {
        QString file_name = QFileDialog::getOpenFileName(this, tr("Open scene"), "./res",
                                                         tr("glTF Scene File (*.gltf)"));
        m_pipeline_config.str_val["scene_file"] = file_name.toStdString();
        m_ui->statusbar->showMessage("Using scene " + file_name + ".");
    }
}

void App::on_actionCornellball_triggered() {
    m_ui->statusbar->showMessage("Using scene Cornellball.");
}
