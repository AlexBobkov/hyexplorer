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
        explicit MainWindow(const DataManagerPtr& dataManager);
        virtual ~MainWindow();

    signals:
        void sceneSelected(const ScenePtr& scene);

    private slots:
        void executeQuery();

        void onMainTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void onSecondTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

        void finishLoadScenes();
        
    protected:
        void closeEvent(QCloseEvent* event) override;

    private:
        void initUi();

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