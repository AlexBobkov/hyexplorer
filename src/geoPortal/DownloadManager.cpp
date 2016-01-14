#include "DownloadManager.hpp"
#include "Storage.hpp"

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QHttpMultiPart>
#include <QAuthenticator>

using namespace portal;

DownloadManager::DownloadManager(const DataManagerPtr& dataManager, QObject* parent) :
QObject(parent),
_dataManager(dataManager)
{
    connect(&dataManager->networkAccessManager(), &QNetworkAccessManager::authenticationRequired, [this](QNetworkReply*, QAuthenticator*)
    {
        qDebug() << "authenticationRequired";
    });
}

DownloadManager::~DownloadManager()
{
}

void DownloadManager::downloadOverview(const ScenePtr& scene)
{
    if (scene->hasOverview())
    {
        QString path = Storage::overviewPath(scene, scene->overviewName());

        if (QFile::exists(path))
        {
            qDebug() << "Overview exists in the local cache";

            _dataManager->showOverview(scene, path);
        }
        else
        {
            QNetworkRequest request(QString("http://virtualglobe.ru/geoportal/%0/overviews/%1").arg(scene->sensor()).arg(scene->overviewName()));
                        
            QNetworkReply* reply = _dataManager->networkAccessManager().get(request);
            connect(reply, &QNetworkReply::finished, this, [reply, this, scene]()
            {
                reply->deleteLater();

                if (reply->error() != QNetworkReply::NoError)
                {
                    qDebug() << "Error " << reply->error() << " " << reply->errorString();
                    return;
                }

                QByteArray data = reply->readAll();
                if (data.isNull() || data.isEmpty())
                {
                    qDebug() << "Reply is null or empty";
                    return;
                }

                QString path = Storage::overviewPath(scene, reply->url().fileName());

                QFile localFile(path);
                localFile.open(QIODevice::WriteOnly);
                localFile.write(data);
                localFile.close();

                _dataManager->showOverview(scene, path);
            });
        }
    }
}

