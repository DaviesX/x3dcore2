/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.7.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QOpenGLWidget>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *action_openfile;
    QAction *actionCornellball;
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QOpenGLWidget *ogl_surface;
    QLabel *label;
    QLabel *label_5;
    QLabel *label_7;
    QSpinBox *spinBox;
    QCheckBox *check_autoexposure;
    QLabel *label_6;
    QDoubleSpinBox *spin_manualexposure;
    QLabel *label_2;
    QLabel *label_8;
    QLabel *label_10;
    QLabel *label_3;
    QLabel *label_9;
    QLabel *label_11;
    QLabel *label_4;
    QLabel *label_12;
    QPushButton *button_render;
    QPushButton *button_save;
    QComboBox *combo_tracer;
    QComboBox *combo_structure;
    QMenuBar *menubar;
    QMenu *menuFile;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName(QStringLiteral("MainWindow"));
        MainWindow->resize(1125, 633);
        action_openfile = new QAction(MainWindow);
        action_openfile->setObjectName(QStringLiteral("action_openfile"));
        actionCornellball = new QAction(MainWindow);
        actionCornellball->setObjectName(QStringLiteral("actionCornellball"));
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 9, 0);
        ogl_surface = new QOpenGLWidget(centralwidget);
        ogl_surface->setObjectName(QStringLiteral("ogl_surface"));

        gridLayout->addWidget(ogl_surface, 0, 0, 10, 1);

        label = new QLabel(centralwidget);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 1, 1, 1);

        label_5 = new QLabel(centralwidget);
        label_5->setObjectName(QStringLiteral("label_5"));

        gridLayout->addWidget(label_5, 1, 1, 1, 1);

        label_7 = new QLabel(centralwidget);
        label_7->setObjectName(QStringLiteral("label_7"));

        gridLayout->addWidget(label_7, 2, 1, 1, 1);

        spinBox = new QSpinBox(centralwidget);
        spinBox->setObjectName(QStringLiteral("spinBox"));
        spinBox->setValue(16);

        gridLayout->addWidget(spinBox, 2, 2, 1, 1);

        check_autoexposure = new QCheckBox(centralwidget);
        check_autoexposure->setObjectName(QStringLiteral("check_autoexposure"));
        check_autoexposure->setChecked(true);

        gridLayout->addWidget(check_autoexposure, 3, 1, 1, 1);

        label_6 = new QLabel(centralwidget);
        label_6->setObjectName(QStringLiteral("label_6"));

        gridLayout->addWidget(label_6, 4, 1, 1, 1);

        spin_manualexposure = new QDoubleSpinBox(centralwidget);
        spin_manualexposure->setObjectName(QStringLiteral("spin_manualexposure"));
        spin_manualexposure->setEnabled(false);
        spin_manualexposure->setValue(1);

        gridLayout->addWidget(spin_manualexposure, 4, 2, 1, 1);

        label_2 = new QLabel(centralwidget);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 5, 1, 1, 1);

        label_8 = new QLabel(centralwidget);
        label_8->setObjectName(QStringLiteral("label_8"));

        gridLayout->addWidget(label_8, 5, 2, 1, 1);

        label_10 = new QLabel(centralwidget);
        label_10->setObjectName(QStringLiteral("label_10"));

        gridLayout->addWidget(label_10, 5, 3, 1, 1);

        label_3 = new QLabel(centralwidget);
        label_3->setObjectName(QStringLiteral("label_3"));

        gridLayout->addWidget(label_3, 6, 1, 1, 1);

        label_9 = new QLabel(centralwidget);
        label_9->setObjectName(QStringLiteral("label_9"));

        gridLayout->addWidget(label_9, 6, 2, 1, 1);

        label_11 = new QLabel(centralwidget);
        label_11->setObjectName(QStringLiteral("label_11"));

        gridLayout->addWidget(label_11, 6, 3, 1, 1);

        label_4 = new QLabel(centralwidget);
        label_4->setObjectName(QStringLiteral("label_4"));

        gridLayout->addWidget(label_4, 7, 1, 1, 1);

        label_12 = new QLabel(centralwidget);
        label_12->setObjectName(QStringLiteral("label_12"));

        gridLayout->addWidget(label_12, 7, 2, 1, 1);

        button_render = new QPushButton(centralwidget);
        button_render->setObjectName(QStringLiteral("button_render"));

        gridLayout->addWidget(button_render, 8, 3, 1, 1);

        button_save = new QPushButton(centralwidget);
        button_save->setObjectName(QStringLiteral("button_save"));

        gridLayout->addWidget(button_save, 9, 3, 1, 1);

        combo_tracer = new QComboBox(centralwidget);
        combo_tracer->setObjectName(QStringLiteral("combo_tracer"));

        gridLayout->addWidget(combo_tracer, 0, 2, 1, 2);

        combo_structure = new QComboBox(centralwidget);
        combo_structure->setObjectName(QStringLiteral("combo_structure"));

        gridLayout->addWidget(combo_structure, 1, 2, 1, 2);

        gridLayout->setColumnStretch(0, 1);
        gridLayout->setColumnMinimumWidth(0, 800);
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 1125, 22));
        menuFile = new QMenu(menubar);
        menuFile->setObjectName(QStringLiteral("menuFile"));
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menuFile->menuAction());
        menuFile->addSeparator();
        menuFile->addAction(action_openfile);
        menuFile->addSeparator();
        menuFile->addAction(actionCornellball);

        retranslateUi(MainWindow);

        combo_tracer->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QApplication::translate("MainWindow", "e8yescg", Q_NULLPTR));
        action_openfile->setText(QApplication::translate("MainWindow", "Open", Q_NULLPTR));
        actionCornellball->setText(QApplication::translate("MainWindow", "Cornellball", Q_NULLPTR));
        label->setText(QApplication::translate("MainWindow", "tracer", Q_NULLPTR));
        label_5->setText(QApplication::translate("MainWindow", "structure", Q_NULLPTR));
        label_7->setText(QApplication::translate("MainWindow", "spp", Q_NULLPTR));
        check_autoexposure->setText(QApplication::translate("MainWindow", "auto exposure", Q_NULLPTR));
        label_6->setText(QApplication::translate("MainWindow", "exposure", Q_NULLPTR));
        label_2->setText(QApplication::translate("MainWindow", "sample deviation", Q_NULLPTR));
        label_8->setText(QApplication::translate("MainWindow", "N/A", Q_NULLPTR));
        label_10->setText(QApplication::translate("MainWindow", "N/A", Q_NULLPTR));
        label_3->setText(QApplication::translate("MainWindow", "scaled deviation", Q_NULLPTR));
        label_9->setText(QApplication::translate("MainWindow", "N/A", Q_NULLPTR));
        label_11->setText(QApplication::translate("MainWindow", "N/A", Q_NULLPTR));
        label_4->setText(QApplication::translate("MainWindow", "time", Q_NULLPTR));
        label_12->setText(QApplication::translate("MainWindow", "N/A", Q_NULLPTR));
        button_render->setText(QApplication::translate("MainWindow", "start", Q_NULLPTR));
        button_save->setText(QApplication::translate("MainWindow", "save", Q_NULLPTR));
        combo_tracer->setCurrentText(QString());
        menuFile->setTitle(QApplication::translate("MainWindow", "Scene", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
