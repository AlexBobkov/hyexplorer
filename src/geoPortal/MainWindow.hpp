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
#include <QTableView>


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

    private slots:
        void executeQuery();        

        void showAbout();
        void showMetadataDescription();

        void selectScene(const QModelIndex& index);
        void zoomToScene(const QModelIndex& index);

    signals:
        void sceneSelected(const ScenePtr& scene);

    protected:
        void resizeEvent(QResizeEvent* event) override;

    private:
        void initUi();
        void onMousePositionChanged(const osgEarth::GeoPoint& point);
        void onMouseClicked();

        Ui::MainWindow _ui;

        DataManagerPtr _dataManager;

        osg::ref_ptr<osgGA::GUIEventHandler> _handler;
              
        QDockWidget* _metadataDock;

        QDockWidget* _scenesDock;
        QTableView* _scenesView;

        QDockWidget* _scenes2Dock;
        QTableView* _scenes2View;

        DataSetPtr _dataset;

        osgEarth::GeoPoint _point; //координаты клика мышкой
    };
}