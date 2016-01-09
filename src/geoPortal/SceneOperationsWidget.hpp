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

        void uploadProcessedFileRequested(const ScenePtr& scene, const QString& filepath, int band, double contrast, double sharpness, int blocksize);

    public slots:
        void setScene(const ScenePtr& scene);
        void onRectangleSelected(const osgEarth::Bounds& b);
        void onRectangleSelectFailed();
        void onSceneDownloaded(const ScenePtr& scene, bool result, const QString& message);
        void onProcessedFileUploaded(const ScenePtr& scene, bool result, const QString& message);

    private slots:
        void download();
        void startImageCorrection();
        void openFolder();
        void showTableWithProcessedFiles();
        void downloadProcessedFile();

        void onImageCorrectionError(QProcess::ProcessError error);
        void onImageCorrectionFinished(int exitCode, QProcess::ExitStatus exitStatus);
    
    private:
        void initUi();
        void uploadProccessedFile();

        Ui::SceneOperationsWidget _ui;

        ScenePtr _scene;

        DataManagerPtr _dataManager;
                
        QString _proccessedOutputFilepath;
        ScenePtr _processingScene;
        int _processingBand;
    };
}