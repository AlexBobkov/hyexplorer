/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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