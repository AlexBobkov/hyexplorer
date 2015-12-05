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
_downloadPathIndex(0),
_isClip(false),
_clipNumber(0)
{
    connect(&_networkManager, SIGNAL(finished(QNetworkReply*)), SLOT(onFileDownloaded(QNetworkReply*)));
    connect(&_networkManager, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)), SLOT(onAuthenticationRequired(QNetworkReply*, QAuthenticator*)));
}

DownloadManager::~DownloadManager()
{
}

void DownloadManager::downloadFromUsgs(const ScenePtr& scene)
{
    if (scene->hasScene)
    {
        qDebug() << "Scene is already on the server";
        return;
    }
    
    QNetworkRequest request(QString::fromUtf8("https://ers.cr.usgs.gov/login/"));
           
    request.setAttribute(QNetworkRequest::User, QString("UsgsLogin"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    QString options = "username=AlexBobkov&password=1qaz2wsx";
    
    QNetworkReply* reply = _networkManager.post(request, options.toLocal8Bit());

    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
}

void DownloadManager::downloadOverview(const ScenePtr& scene)
{
    if (scene->hasOverview)
    {
        QString path = makeOverviewPath(*scene->overviewName);

        if (QFile::exists(path))
        {
            qDebug() << "Overview exists in the local cache";

            _dataManager->showOverview(scene, path);
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

void DownloadManager::downloadScene(const ScenePtr& scene, int minBand, int maxBand)
{
    //Нельзя скачивать одновременно 2 сцены
    if (_downloadPaths.size() > 0)
    {
        emit sceneDownloadFinished(scene, false, tr("Сцена %0 не может быть получена, пока происходит получение другой сцены").arg(scene->sceneid));
        return;
    }

    _isClip = false;

    //QNetworkRequest request(QString::fromUtf8("http://localhost:5000/scene/%0/%1/%2").arg(scene->sceneid).arg(minBand).arg(maxBand));
    QNetworkRequest request(QString::fromUtf8("http://virtualglobe.ru/geoportalapi/scene/%0/%1/%2").arg(scene->sceneid).arg(minBand).arg(maxBand));
    request.setAttribute(QNetworkRequest::User, QString("Scene"));

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    _networkManager.get(request);
}

void DownloadManager::downloadSceneClip(const ScenePtr& scene, int minBand, int maxBand)
{
    //Нельзя скачивать одновременно 2 сцены
    if (_downloadPaths.size() > 0)
    {
        emit sceneDownloadFinished(scene, false, tr("Сцена %0 не может быть получена, пока происходит получение другой сцены").arg(scene->sceneid));
        return;
    }

    if (!_dataManager->rectangle())
    {
        emit sceneDownloadFinished(scene, false, tr("Не выбрана область для вырезания"));
        return;
    }

    _isClip = true;

    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();
    QDir clipsDir(dataPath + QString("/hyperion/clips/%0/").arg(scene->sceneid));
    QStringList entries = clipsDir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);
    
    _clipNumber = entries.size();

    qDebug() << "Clip number " << entries << _clipNumber;

    osgEarth::Bounds b = *_dataManager->rectangle();
        
    QNetworkRequest request(QString::fromUtf8("http://virtualglobe.ru/geoportalapi/sceneclip/%0/%1/%2?leftgeo=%3&upgeo=%4&rightgeo=%5&downgeo=%6")
                            .arg(scene->sceneid)
                            .arg(minBand)
                            .arg(maxBand)
                            .arg(b.xMin(), 0, 'f', 10)
                            .arg(b.yMax(), 0, 'f', 10)
                            .arg(b.xMax(), 0, 'f', 10)
                            .arg(b.yMin(), 0, 'f', 10));
    request.setAttribute(QNetworkRequest::User, QString("Scene"));

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    _networkManager.get(request);
}

void DownloadManager::onFileDownloaded(QNetworkReply* reply)
{
    qDebug() << "Downloaded " << qPrintable(reply->url().url());

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    if (!data.isNull() && !data.isEmpty())
    {
        QString requestType = reply->request().attribute(QNetworkRequest::User).toString();
        ScenePtr scene = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).value<ScenePtr>();

        if (requestType == "Overview")
        {
            processOverviewReply(scene, data, reply->url().fileName());
        }
        else if (requestType == "Scene")
        {
            processSceneReply(scene, data);
        }
        else if (requestType == "SceneBand")
        {
            processSceneBandReply(scene, data, reply->url().fileName());
        }        
        else if (requestType == "UsgsRedirect")
        {
            //processSceneBandReply(scene, data, reply->url().fileName());

            QFile localFile("TTT3");
            localFile.open(QIODevice::WriteOnly);
            localFile.write(data);
            localFile.close();
        }
    }
    else
    {
        QString requestType = reply->request().attribute(QNetworkRequest::User).toString();
        ScenePtr scene = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).value<ScenePtr>();

        if (requestType == "UsgsLogin")
        {
            QNetworkRequest request(QString::fromUtf8("http://earthexplorer.usgs.gov/download/1854/EO1H0250382005247110PY_PF1_01/L1T/EE"));

            request.setAttribute(QNetworkRequest::User, QString("UsgsStart"));

            QVariant v;
            v.setValue(scene);
            request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

            _networkManager.get(request);
        }
        else if (requestType == "UsgsStart")
        {
            int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
            if (statusCode != 302)
            {
                qDebug() << "Wrong status code " << statusCode;
                reply->deleteLater();
                return;
            }

            QUrl possibleRedirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
                        
            if (possibleRedirectUrl.isEmpty() || possibleRedirectUrl == _oldRedirectUrl)
            {
                qDebug() << "Wrong url " << possibleRedirectUrl;
                reply->deleteLater();
                return;
            }

            _oldRedirectUrl = possibleRedirectUrl;

            QNetworkRequest request(possibleRedirectUrl);

            request.setAttribute(QNetworkRequest::User, QString("UsgsRedirect"));

            QVariant v;
            v.setValue(scene);
            request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

            _networkManager.get(request);
        }

        qDebug() << "Reply is null or empty";
    }

    reply->deleteLater();
}

void DownloadManager::onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    qDebug() << "authenticationRequired";
}

void DownloadManager::processOverviewReply(const ScenePtr& scene, const QByteArray& data, const QString& filename)
{
    QString path = makeOverviewPath(filename);

    QFile localFile(path);
    if (!localFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file " << qPrintable(path);
        return;
    }
    localFile.write(data);
    localFile.close();

    _dataManager->showOverview(scene, path);
}

void DownloadManager::processSceneReply(const ScenePtr& scene, const QByteArray& data)
{
    QFile file("debug.txt");
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();

    if (!data.startsWith("SUCCESS"))
    {
        _downloadPaths.clear();
        emit sceneDownloadFinished(scene, false, tr("Сцена %0 не найдена на сервере").arg(scene->sceneid));
        return;
    }

    assert(_downloadPaths.size() == 0);
        
    QTextStream stream(data);
    QString line = stream.readLine(); //SUCCESS line
    while (!line.isNull())
    {
        line = stream.readLine();
        if (!line.isNull() && !line.isEmpty())
        {
            _downloadPaths.push_back(line);
        }
    }

    if (_downloadPaths.size() == 0)
    {
        emit sceneDownloadFinished(scene, false, tr("Каналы для сцены %0 не найдены").arg(scene->sceneid));
        return;
    }

    _downloadPathIndex = 0;
    downloadNextSceneBand(scene);
}

void DownloadManager::processSceneBandReply(const ScenePtr& scene, const QByteArray& data, const QString& filename)
{
    qDebug() << "SceneBand " << _downloadPathIndex;

    QString path = makeSceneBandPath(scene, filename);    

    QFile localFile(path);
    if (!localFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file " << qPrintable(path);
        return;
    }
    localFile.write(data);
    localFile.close();

    _downloadPathIndex++;
    if (_downloadPathIndex >= _downloadPaths.size())
    {
        _downloadPaths.clear();
        emit sceneDownloadFinished(scene, true, tr("Сцена %0 успешно получена").arg(scene->sceneid));
        return;
    }

    downloadNextSceneBand(scene);
}

void DownloadManager::downloadNextSceneBand(const ScenePtr& scene)
{
    assert(_downloadPathIndex < _downloadPaths.size());

    emit progressChanged(0);

    QNetworkRequest request(_downloadPaths[_downloadPathIndex]);

    QString path = makeSceneBandPath(scene, request.url().fileName());
    if (QFile::exists(path))
    {
        _downloadPathIndex++;
        if (_downloadPathIndex >= _downloadPaths.size())
        {
            _downloadPaths.clear();
            emit sceneDownloadFinished(scene, true, tr("Сцена %0 успешно получена").arg(scene->sceneid));
            return;
        }

        downloadNextSceneBand(scene);
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

QString DownloadManager::makeOverviewPath(const QString& filename)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName("hyperion/overviews/");
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }
        
    return dataDir.filePath(folderName + filename);
}

QString DownloadManager::makeSceneBandPath(const ScenePtr& scene, const QString& filename)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    
    QString folderName;
    if (_isClip)
    {
        folderName = QString("hyperion/clips/%0/clip%1/").arg(scene->sceneid).arg(_clipNumber);
    }
    else
    {
        folderName = QString("hyperion/scenes/%0/").arg(scene->sceneid);
    }

    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}