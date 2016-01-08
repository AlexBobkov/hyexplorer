#pragma once

#include "DataManager.hpp"
#include "Scene.hpp"

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
        void uploadProcessedFileFinished(const ScenePtr& scene, bool result, const QString& message);

        void progressChanged(int);
        
    public slots:
        void importScene(const ScenePtr& scene);
        void downloadOverview(const ScenePtr& scene);
        void downloadScene(const ScenePtr& scene, int minBand, int maxBand);
        void downloadSceneClip(const ScenePtr& scene, int minBand, int maxBand);
        void uploadProcessedFile(const ScenePtr& scene, const QString& filepath, int band, double contrast, double sharpness, int blocksize);
        
    private slots:        
        void onFileDownloaded(QNetworkReply* reply);
        void onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);        
        void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void cancelDownload();
        void readDataChunk();
                    
    private:
        void processOverviewReply(const ScenePtr& scene, QNetworkReply* reply);
        void processSceneReply(const ScenePtr& scene, QNetworkReply* reply);
        void processSceneBandReply(const ScenePtr& scene, QNetworkReply* reply);
        void processImportLoginReply(const ScenePtr& scene, QNetworkReply* reply);
        void processImportFirstReply(const ScenePtr& scene, QNetworkReply* reply);
        void processImportRedirectReply(const ScenePtr& scene, QNetworkReply* reply);
        void processUploadReply(const ScenePtr& scene, QNetworkReply* reply);
        void processUploadProcessedReply(const ScenePtr& scene, QNetworkReply* reply);

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