#pragma once

#include "ui_ProcessingWidget.h"
#include "DataManager.hpp"

#include <QWidget>
#include <QProcess>

namespace portal
{
    class ProcessingWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit ProcessingWidget(const DataManagerPtr& dataManager, QWidget* parent = 0);        
        virtual ~ProcessingWidget();

    signals:
        void processingStarted();
        void processingFinished();

    public slots:
        void setSceneAndClip(const ScenePtr& scene, const ClipInfoPtr& clipInfo);

    private slots:        
        void startImageCorrection();        
        void showTableWithProcessedFiles();
        void downloadProcessedFile(const QString& filename);
            
    private:
        void initUi();
        void uploadProccessedFile();

        Ui::ProcessingWidget _ui;

        DataManagerPtr _dataManager;

        QStringList _tools;

        ScenePtr _scene;
        ClipInfoPtr _clipInfo;        
                
        QString _proccessedOutputFilepath;
        QString _appName;
    };
}