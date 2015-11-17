#pragma once

#include "DataManager.hpp"
#include "Scene.hpp"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

namespace portal
{
    class DownloadManager : public QObject
    {
        Q_OBJECT

    public:
        explicit DownloadManager(const DataManagerPtr& dataManager, QObject* parent = 0);
        virtual ~DownloadManager();

    signals:
        void finished();

    public slots:
        void downloadOverview(const ScenePtr& scene);
        
    private slots:        
        void onFileDownloaded(QNetworkReply* reply);
        void onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);
        void downloadScene(const ScenePtr& scene, int minBand, int maxBand);
        void downloadSceneBand(const ScenePtr& scene);
    
    private:
        DataManagerPtr _dataManager;

        QNetworkAccessManager _networkManager;
                
        QStringList _downloadPaths;
        int _downloadPathIndex;
    };
}