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
        void onSceneDownloaded(const ScenePtr& scene, bool result, const QString& message);
        
    private slots:
        void onMinimumBandChanged(int i);
        void onMaximumBandChanged(int i);
        void onGlobeBandChanged(int i);
        void onFragmentRadioButtonToggled(bool b);
        void selectRectangle(bool b);
        void onRectangleBoundsChanged(double d);
        void importScene();
        void download();
        void startImageCorrection();
        void openFolder();
        void showBandOnGlobe();

        void onImageCorrectionError(QProcess::ProcessError error);
        void onImageCorrectionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
    private:
        void initUi();

        Ui::SceneOperationsWidget _ui;

        ScenePtr _scene;

        DataManagerPtr _dataManager;
    };
}