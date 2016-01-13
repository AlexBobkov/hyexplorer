#pragma once

#include "Scene.hpp"

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>

namespace portal
{
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