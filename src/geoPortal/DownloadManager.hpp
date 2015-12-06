#pragma once

#include "DataManager.hpp"
#include "Scene.hpp"

#include <QNetworkAccessManager>
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
        void usgsDownloadFinished(const ScenePtr& scene, bool result, const QString& message);

        void progressChanged(int);
        
    public slots:
        void downloadFromUsgs(const ScenePtr& scene);
        void downloadOverview(const ScenePtr& scene);
        void downloadScene(const ScenePtr& scene, int minBand, int maxBand);
        void downloadSceneClip(const ScenePtr& scene, int minBand, int maxBand);
        
    private slots:        
        void onFileDownloaded(QNetworkReply* reply);
        void onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator);        
        void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
        void cancelDownload();
        void readDataChunk();
                    
    private:
        void processOverviewReply(const ScenePtr& scene, const QByteArray& data, const QString& filename);
        void processSceneReply(const ScenePtr& scene, const QByteArray& data);
        void processSceneBandReply(const ScenePtr& scene, const QByteArray& data, const QString& filename);
        void processUsgsLoginReply(const ScenePtr& scene);
        void processUsgsFirstReply(const ScenePtr& scene, QNetworkReply* reply);
        void processUsgsRedirectReply(const ScenePtr& scene, QNetworkReply* reply);

        void downloadNextSceneBand(const ScenePtr& scene);

        QString makeOverviewPath(const QString& filename);
        QString makeSceneBandPath(const ScenePtr& scene, const QString& filename);

        DataManagerPtr _dataManager;

        QNetworkAccessManager _networkManager;
                
        QStringList _downloadPaths;
        int _downloadPathIndex;
        bool _isClip;
        int _clipNumber;

        QUrl _oldRedirectUrl;

        QProgressDialog* _progressDialog;
        QNetworkReply* _longDownloadReply;
        QFile* _tempFile;
    };
}