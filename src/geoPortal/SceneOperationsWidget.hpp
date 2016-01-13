#pragma once

#include "ui_SceneOperationsWidget.h"
#include "DataManager.hpp"

#include <QWidget>
#include <QProcess>

namespace portal
{
    class SceneOperationsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit SceneOperationsWidget(const DataManagerPtr& dataManager, QWidget* parent = 0);        
        virtual ~SceneOperationsWidget();

    signals:
        void downloadSceneRequested(const ScenePtr& scene, const ClipInfoPtr&);
        void importSceneRequested(const ScenePtr& scene);

        void selectRectangleRequested();
        void rectangleChanged(const osgEarth::Bounds& b);

        void sceneClipPrepared(const ScenePtr& scene, const ClipInfoPtr&);
        
    public slots:
        void setScene(const ScenePtr& scene);
        void onRectangleSelected(const osgEarth::Bounds& b);
        void onRectangleSelectFailed();
        void onSceneDownloaded(const ScenePtr& scene, bool result, const QString& message);
                    
    private:
        void initUi();
        
        Ui::SceneOperationsWidget _ui;

        ScenePtr _scene;
        ClipInfoPtr _clipInfo;

        DataManagerPtr _dataManager;
    };
}