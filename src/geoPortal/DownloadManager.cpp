#include "DownloadManager.hpp"

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QApplication>

using namespace portal;

DownloadManager::DownloadManager(const DataManagerPtr& dataManager, QObject* parent) :
QObject(parent),
_dataManager(dataManager),
_downloadPathIndex(0)
{
    connect(&_networkManager, SIGNAL(finished(QNetworkReply*)), SLOT(onFileDownloaded(QNetworkReply*)));
    connect(&_networkManager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), SLOT(onAuthenticationRequired(QNetworkReply*, QAuthenticator*)));
}

DownloadManager::~DownloadManager()
{
}

void DownloadManager::downloadOverview(const ScenePtr& scene)
{
    if (scene->hasOverview)
    {
        QSettings settings;
        QString dataPath = settings.value("StoragePath").toString();
        QString overviewFilepath = dataPath + QString("/overviews/") + *scene->overviewName;

        if (QFile::exists(overviewFilepath))
        {
            qDebug() << "Overview exists in the local cache";

            _dataManager->showOverview(scene, overviewFilepath);
        }
        else
        {
            QNetworkRequest request(QString::fromUtf8("http://virtualglobe.ru/geoportal/hyperion/overviews/%0").arg(*scene->overviewName));

            request.setAttribute(QNetworkRequest::User, QString("Overview"));

            QVariant v;
            v.setValue(scene);
            request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

            _networkManager.get(request);
        }
    }
}

void DownloadManager::onFileDownloaded(QNetworkReply* reply)
{
    qDebug() << "Downloaded " << qPrintable(reply->url().url());

    QByteArray data = reply->readAll();
    if (!data.isNull() && !data.isEmpty())
    {
        QString requestType = reply->request().attribute(QNetworkRequest::User).toString();
        ScenePtr scene = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).value<ScenePtr>();

        if (requestType == "Overview")
        {
            QSettings settings;
            QString dataPath = settings.value("StoragePath").toString();
            QDir dataDir(dataPath);
            if (!dataDir.exists("overviews"))
            {
                dataDir.mkpath("overviews");
            }

            QString overviewFilepath = dataDir.filePath(QString("overviews/") + reply->url().fileName());

            QFile localFile(overviewFilepath);
            if (!localFile.open(QIODevice::WriteOnly))
            {
                qDebug() << "Failed to open file " << qPrintable(overviewFilepath);
                return;
            }

            localFile.write(data);
            localFile.close();

            _dataManager->showOverview(scene, overviewFilepath);
        }
        else if (requestType == "Scene")
        {
            if (data.startsWith("SUCCESS"))
            {
                qDebug() << "Success " << scene->sceneid;

                _downloadPaths.clear();

                QTextStream stream(data);
                QString line = stream.readLine(); //SUCCESS
                while (!line.isNull())
                {
                    line = stream.readLine();
                    if (!line.isNull() && !line.isEmpty())
                    {
                        _downloadPaths.push_back(line);
                    }
                }

                _downloadPathIndex = 0;

                downloadSceneBand(scene);
            }
            else
            {
                QMessageBox::warning(qApp->activeWindow(), tr("ќшибка получени€ сцены"), tr("—цена %0 не найдена на сервере").arg(scene->sceneid));
            }

            QFile file("debug.txt");
            file.open(QIODevice::WriteOnly);
            file.write(data);
            file.close();
        }
        else if (requestType == "SceneBand")
        {
            qDebug() << "SceneBand " << _downloadPathIndex;

            QSettings settings;
            QString dataPath = settings.value("StoragePath").toString();
            QDir dataDir(dataPath);
            QString folderName = QString("hyperion/scenes/%0/").arg(scene->sceneid);
            if (!dataDir.exists(folderName))
            {
                dataDir.mkpath(folderName);
            }

            QString bandFilepath = dataDir.filePath(folderName + reply->url().fileName());

            QFile localFile(bandFilepath);
            if (!localFile.open(QIODevice::WriteOnly))
            {
                qDebug() << "Failed to open file " << qPrintable(bandFilepath);
                return;
            }

            localFile.write(data);
            localFile.close();

            _downloadPathIndex++;
            if (_downloadPathIndex >= _downloadPaths.size())
            {
                return;
            }

            downloadSceneBand(scene);
        }
    }

    reply->deleteLater();
}

void DownloadManager::onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    qDebug() << "authenticationRequired";
}

void DownloadManager::downloadScene(const ScenePtr& scene, int minBand, int maxBand)
{
    qDebug() << "Download " << scene->sceneid;

    //QNetworkRequest request(QString::fromUtf8("http://localhost:5000/scene/%0/%1/%2").arg(scene->sceneid).arg(minBand).arg(maxBand));
    QNetworkRequest request(QString::fromUtf8("http://178.62.140.44:5000/scene/%0/%1/%2").arg(scene->sceneid).arg(minBand).arg(maxBand));
    request.setAttribute(QNetworkRequest::User, QString("Scene"));

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    _networkManager.get(request);
}

void DownloadManager::downloadSceneBand(const ScenePtr& scene)
{
    if (_downloadPaths.size() == 0 || _downloadPathIndex >= _downloadPaths.size())
    {
        return;
    }

    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();
    QDir dataDir(dataPath);
    QString folderName = QString("hyperion/scenes/%0/").arg(scene->sceneid);
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    QNetworkRequest request(_downloadPaths[_downloadPathIndex]);
    if (dataDir.exists(folderName + request.url().fileName()))
    {
        _downloadPathIndex++;

        if (_downloadPathIndex >= _downloadPaths.size())
        {
            return;
        }

        downloadSceneBand(scene);
    }
    else
    {
        request.setAttribute(QNetworkRequest::User, QString("SceneBand"));

        QVariant v;
        v.setValue(scene);
        request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

        _networkManager.get(request);
    }
}