#pragma once

#include "ui_SceneOperationsWidget.h"
#include "DataManager.hpp"

#include <QWidget>

namespace portal
{
    class SceneOperationsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit SceneOperationsWidget(const DataManagerPtr& dataManager, QWidget* parent = 0);        
        virtual ~SceneOperationsWidget();

    signals:
        void downloadSceneRequested(const ScenePtr& scene, int minBand, int maxBand);
        void downloadSceneClipRequested(const ScenePtr& scene, int minBand, int maxBand);

        void importSceneRequested(const ScenePtr& scene);

        void selectRectangleRequested();
        void rectangleChanged(const osgEarth::Bounds& b);

    public slots:
        void setScene(const ScenePtr& scene);
        void onRectangleSelected(const osgEarth::Bounds& b);
        void onRectangleSelectFailed();
        void onSceneImported(const ScenePtr& scene);
        
    private slots:
        void onMinimumBandChanged(int i);
        void onMaximumBandChanged(int i);
        void onGlobeBandChanged(int i);        
        void onFragmentRadioButtonToggled(bool b);
        void selectRectangle(bool b);
        void onRectangleBoundsChanged(double d);
        void importScene();
        void download();
    
    private:
        void initUi();

        Ui::SceneOperationsWidget _ui;

        ScenePtr _scene;

        DataManagerPtr _dataManager;
    };
}