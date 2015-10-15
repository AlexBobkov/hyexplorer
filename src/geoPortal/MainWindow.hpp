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
    void executeQuery();
    void updateLayer(const std::string& query);

    void showAbout();
    void showMetadataDescription();

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void initUi();

    Ui::MainWindow _ui;

    osg::ref_ptr<osgEarth::MapNode> _mapNode;

    osg::observer_ptr<osgEarth::ModelLayer> _oldLayer;
    std::string _oldQuery;
};
