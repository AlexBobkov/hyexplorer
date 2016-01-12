﻿#pragma once

#include "DataManager.hpp"
#include "Scene.hpp"
#include "ClipInfo.hpp"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QFile>

namespace portal
{
    class DownloadManager : public QObject
    {
        Q_OBJECT

    public:
        explicit DownloadManager(const DataManagerPtr& dataManager, QObject* parent = 0);
        virtual ~DownloadManager();

    signals:
        void sceneDownloadFinished(const ScenePtr& scene, bool result, const QString& message);
        void importFinished(const ScenePtr& scene, bool result, const QString& message);

        void progressChanged(int);
        
    public slots:
        void importScene(const ScenePtr& scene);
        void downloadOverview(const ScenePtr& scene);
        void downloadScene(const ScenePtr& scene, const ClipInfoPtr& clipInfo);
                   
    private:        
        void processRedirectReply();
        void processDownloadReply();
        void downloadNextSceneBand(const ScenePtr& scene, const ClipInfoPtr& clipInfo);
        
        DataManagerPtr _dataManager;
                                
        QStringList _downloadPaths;
        int _downloadPathIndex;

        QUrl _oldRedirectUrl;

        QProgressDialog* _progressDialog;
        QNetworkReply* _longDownloadReply;
        QFile* _tempFile;
    };
}