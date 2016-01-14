#pragma once

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
     
    public slots:
        void downloadOverview(const ScenePtr& scene);
                   
    private:
        DataManagerPtr _dataManager;
    };
}