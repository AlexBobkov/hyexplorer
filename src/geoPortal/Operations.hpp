/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
    class DownloadOverviewOperation : public QObject
    {
        Q_OBJECT

    public:
        explicit DownloadOverviewOperation(const ScenePtr& scene, QNetworkAccessManager* manager, QObject* parent = 0);
        virtual ~DownloadOverviewOperation() {}

        void start();

    signals:
        void finished(const ScenePtr& scene, const QString& filepath);
        void error(const QString& text);

    private:
        QNetworkAccessManager* _networkManager;

        ScenePtr _scene;
    };

    class DownloadSceneOperation : public QObject
    {
        Q_OBJECT

    public:
        explicit DownloadSceneOperation(const ScenePtr& scene, const ClipInfoPtr& clipInfo, QNetworkAccessManager* manager, QObject* parent = 0);
        virtual ~DownloadSceneOperation() {}

        void start();

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
        virtual ~ImportSceneOperation() {}
        
        void start();

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
        virtual ~ProcessingOperation() {}

        void start();

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