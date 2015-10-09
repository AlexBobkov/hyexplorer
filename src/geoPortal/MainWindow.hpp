#pragma once

#include "ui_MainWindow.h"

#include <osgEarth/MapNode>

#include <QtGui>
#include <QMainWindow>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow();
    virtual ~MainWindow();

    void setMapNode(osgEarth::MapNode* mapNode);

public slots:
    void loadScenes();

private:
    void initUi();

    Ui::MainWindow _ui;

    osg::ref_ptr<osgEarth::MapNode> _mapNode;
};
