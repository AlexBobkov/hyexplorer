#pragma once

#include "ui_MainWindow.h"
#include "Scene.hpp"
#include "DataManager.hpp"

#include <osgGA/GUIEventHandler>
#include <osgViewer/View>
#include <osgEarth/Bounds>
#include <osgEarth/GeoData>
#include <osgEarth/MapNode>
#include <osgEarthFeatures/Feature>

#include <QtGui>
#include <QMainWindow>
#include <QLabel>
#include <QTabWidget>
#include <QPushButton>
#include <QTableView>
#include <QProgressBar>
#include <QMoveEvent>

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

    signals:
        void sceneSelected(const ScenePtr& scene);
        void rectangleSelected(const osgEarth::Bounds& b);
        void rectangleSelectFailed();

    private slots:
        void executeQuery();

        void selectScene(const QModelIndex& index);
        void zoomToScene(const QModelIndex& index);

        void onMainTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void onSecondTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

        void finishLoadScenes();

        void onRectangleSelected(const osgEarth::Bounds& bounds);
        void onRectangleSelectionFailed();

        void sensorChanged();

    protected:
        void closeEvent(QCloseEvent* event) override;

    private:
        void initUi();

        void onMousePositionChanged(const osgEarth::GeoPoint& point);
        void onMouseClicked();        

        void loadScenes();

        Ui::MainWindow _ui;

        QProgressBar* _progressBar;

        DataManagerPtr _dataManager;

        osg::ref_ptr<osgGA::GUIEventHandler> _handler;
        
        QTableView* _scenesMainView;
        QTableView* _scenesSecondView;

        QLabel* _mousePosLabel;

        DataSetPtr _dataset;

        osgEarth::GeoPoint _point; //координаты клика мышкой
    };
}