#pragma once

#include <QtGui>
#include <QMainWindow>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>

#include "ui_MainWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow();
    virtual ~MainWindow();

public slots:
    void test();

private:
    void initUi();

    Ui::MainWindow _ui;
};
