#include "MainWindow.hpp"

#include <QAction>
#include <QDockWidget>
#include <QDesktopServices>
#include <QLocale>
#include <QtGui>
#include <QMainWindow>
#include <QToolBar>
#include <QWidgetAction>
#include <QFileDialog>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

MainWindow::MainWindow() :
QMainWindow()
{
    initUi();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUi()
{
    _ui.setupUi(this);
}

void MainWindow::test()
{
}