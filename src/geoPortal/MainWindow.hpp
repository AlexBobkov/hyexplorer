#pragma once

#include "ui_MainWindow.h"
#include "Scene.hpp"
#include "DataManager.hpp"
#include "EventHandlers.hpp"

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

    private slots:
        void executeQuery();

        void selectScene(const QModelIndex& index);
        void zoomToScene(const QModelIndex& index);

        void onMainTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void onSecondTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

        void finishLoadScenes();

        void sensorChanged();

    protected:
        void closeEvent(QCloseEvent* event) override;

    private:
        void initUi();
        void loadScenes();

        Ui::MainWindow _ui;

        QProgressBar* _progressBar;

        DataManagerPtr _dataManager;

        osg::ref_ptr<ReportMoveMouseHandler> _mouseReportHandler;
        osg::ref_ptr<ReportClickMouseHandler> _sceneSelectHandler;
        osg::ref_ptr<ReportClickMouseHandler> _centerSelectHandler;

        QTableView* _scenesMainView;
        QTableView* _scenesSecondView;

        QLabel* _mousePosLabel;

        DataSetPtr _dataset;
    };
}