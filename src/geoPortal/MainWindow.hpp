#pragma once

#include "ui_MainWindow.h"
#include "Scene.hpp"
#include "DataManager.hpp"
#include "DownloadManager.hpp"

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

    private slots:
        void executeQuery();

        void showAbout();
        void showMetadataDescription();
        void showSettings();

        void selectScene(const QModelIndex& index);
        void zoomToScene(const QModelIndex& index);

        void onMainTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
        void onSecondTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);

        void finishLoadScenes();
        void finishLoadBands(const ScenePtr& scene, bool result, const QString& message);

    signals:
        void sceneSelected(const ScenePtr& scene);

    protected:
        void moveEvent(QMoveEvent* event) override;
        void resizeEvent(QResizeEvent* event) override;

    private:
        void initUi();

        void onMousePositionChanged(const osgEarth::GeoPoint& point);
        void onMouseClicked();

        void loadScenes();

        Ui::MainWindow _ui;

        QProgressBar* _progressBar;

        DataManagerPtr _dataManager;

        osg::ref_ptr<osgGA::GUIEventHandler> _handler;

        QDockWidget* _sceneWidgetDock;

        QDockWidget* _scenesMainDock;
        QTableView* _scenesMainView;

        QDockWidget* _scenesSecondDock;
        QTableView* _scenesSecondView;

        DownloadManager* _downloadManager;

        DataSetPtr _dataset;

        osgEarth::GeoPoint _point; //координаты клика мышкой
    };
}