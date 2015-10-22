#pragma once

#include "ui_MainWindow.h"
#include "Scene.hpp"
#include "DataManager.hpp"

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

        void setDataManager(const DataManagerPtr& dataManager);
        
        void setScene(const ScenePtr& scene);

    public slots:
        void executeQuery();        

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

        DataManagerPtr _dataManager;

        osg::ref_ptr<osgGA::GUIEventHandler> _handler;

        osg::ref_ptr<osg::Node> _featureNode;
                
        QDockWidget* _metadataDock;        
    };
}