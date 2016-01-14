#pragma once

#include "Scene.hpp"
#include "ClipInfo.hpp"

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QProgressDialog>
#include <QFile>

namespace portal
{
    class DownloadSceneOperation : public QObject
    {
        Q_OBJECT

    public:
        explicit DownloadSceneOperation(const ScenePtr& scene, const ClipInfoPtr& clipInfo, QNetworkAccessManager* manager, QObject* parent = 0);
        virtual ~DownloadSceneOperation();

    signals:
        void finished(const ScenePtr& scene, const ClipInfoPtr& clipInfo);
        void error(const QString& text);

        void progressChanged(int);

    private:
        void downloadNextSceneBand();

        QNetworkAccessManager* _networkManager;

        QStringList _downloadPaths;
        int _downloadPathIndex;

        ScenePtr _scene;
        ClipInfoPtr _clipInfo;
    };

    class ImportSceneOperation : public QObject
    {
        Q_OBJECT

    public:
        explicit ImportSceneOperation(const ScenePtr& scene, QNetworkAccessManager* manager, QObject* parent = 0);
        virtual ~ImportSceneOperation();

    signals:
        void finished(const ScenePtr& scene);
        void error(const QString& text);

        void progressChanged(int);

    private:
        void downloadScene();
        void uploadScene();

        QNetworkAccessManager* _networkManager;
                
        ScenePtr _scene;

        QFile* _tempFile;
        QNetworkReply* _downloadReply;
        QNetworkReply* _uploadReply;
        QProgressDialog* _progressDialog;
    };

    class ProcessingOperation : public QObject
    {
        Q_OBJECT

    public:
        explicit ProcessingOperation(const ScenePtr& scene, int band, const QString& toolFilepath, const QString& inputFilepath, const QString& outputFilepath, QNetworkAccessManager* manager, QObject* parent = 0);
        virtual ~ProcessingOperation();

    signals:
        void finished();
        void error(const QString& text);

    private:
        void uploadProccessedFile();

        QNetworkAccessManager* _networkManager;

        ScenePtr _scene;
        int _band;
        QString _toolFilepath;
        QString _inputFilepath;
        QString _outputFilepath;

        QProcess* _process;
    };
}