#pragma once

#include "ui_MainWindow.h"
#include "Scene.hpp"

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <osgEarth/GeoData>
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
        void setView(osgViewer::View* view);

        void setScene(const ScenePtr& scene);

    public slots:
        void executeQuery();
        void updateLayer(const std::string& query);

        void selectPoint(bool b);

        void showAbout();
        void showMetadataDescription();

    signals:
        void sceneSelected(const ScenePtr& scene);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        void initUi();
        void setPoint(const osgEarth::GeoPoint& point);

        Ui::MainWindow _ui;

        osg::observer_ptr<osgEarth::MapNode> _mapNode;
        osg::observer_ptr<osgViewer::View> _view;
        osg::ref_ptr<osgGA::GUIEventHandler> _handler;

        osg::ref_ptr<osg::Node> _featureNode;

        osg::observer_ptr<osgEarth::ModelLayer> _oldLayer;
        std::string _oldQuery;

        QDockWidget* _metadataDock;        
    };
}