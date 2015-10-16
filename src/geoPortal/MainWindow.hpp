#pragma once

#include "ui_MainWindow.h"
#include "Scene.hpp"

#include <osgEarth/MapNode>
#include <osgEarthFeatures/Feature>

#include <QtGui>
#include <QMainWindow>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>

namespace portal
{
    class MainWindow : public QMainWindow
    {
        Q_OBJECT

    public:
        explicit MainWindow();
        virtual ~MainWindow();

        void setMapNode(osgEarth::MapNode* mapNode);

        void setScene(const ScenePtr& scene);

    public slots:
        void executeQuery();
        void updateLayer(const std::string& query);

        void showAbout();
        void showMetadataDescription();

    signals:
        void sceneSelected(const ScenePtr& scene);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        void initUi();

        Ui::MainWindow _ui;

        osg::ref_ptr<osgEarth::MapNode> _mapNode;

        osg::observer_ptr<osgEarth::ModelLayer> _oldLayer;
        std::string _oldQuery;

        QDockWidget* _metadataDock;
    };
}