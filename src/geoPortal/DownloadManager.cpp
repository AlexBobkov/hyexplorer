#include "DownloadManager.hpp"
#include "Storage.hpp"

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QApplication>
#include <QHttpMultiPart>

using namespace portal;

DownloadManager::DownloadManager(const DataManagerPtr& dataManager, QObject* parent) :
QObject(parent),
_dataManager(dataManager),
_downloadPathIndex(0),
_isClip(false),
_clipNumber(0),
_progressDialog(0),
_longDownloadReply(0),
_tempFile(0)
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
        QString path = Storage::overviewPath(*scene->overviewName);

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

    QString requestType = reply->request().attribute(QNetworkRequest::User).toString();
    ScenePtr scene = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).value<ScenePtr>();
        
    if (requestType == "Overview")
    {
        processOverviewReply(scene, reply);
    }
    else if (requestType == "Scene")
    {
        processSceneReply(scene, reply);
    }
    else if (requestType == "SceneBand")
    {
        processSceneBandReply(scene, reply);
    }
    else if (requestType == "UsgsLogin")
    {
        processUsgsLoginReply(scene, reply);
    }
    else if (requestType == "UsgsFirst")
    {
        processUsgsFirstReply(scene, reply);
    }
    else if (requestType == "UsgsRedirect")
    {
        processUsgsRedirectReply(scene, reply);
    }
    else if (requestType == "Upload")
    {
        processUploadReply(scene, reply);
    }
    
    reply->deleteLater();
}

void DownloadManager::onAuthenticationRequired(QNetworkReply* reply, QAuthenticator* authenticator)
{
    qDebug() << "authenticationRequired";
}

void DownloadManager::processOverviewReply(const ScenePtr& scene, QNetworkReply* reply)
{
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

    QString path = Storage::overviewPath(reply->url().fileName());

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

void DownloadManager::processSceneReply(const ScenePtr& scene, QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();

        emit sceneDownloadFinished(scene, false, tr("При скачивании сцены %0 произошла ошибка %1 %2").arg(scene->sceneid).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    if (data.isNull() || data.isEmpty())
    {        
        qDebug() << "Reply is null or empty";

        emit sceneDownloadFinished(scene, false, tr("При скачивании сцены %0 получен пустой ответ").arg(scene->sceneid));
        return;
    }

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

void DownloadManager::processSceneBandReply(const ScenePtr& scene, QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();

        emit sceneDownloadFinished(scene, false, tr("При скачивании сцены %0 произошла ошибка %1 %2").arg(scene->sceneid).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    QByteArray data = reply->readAll();
    if (data.isNull() || data.isEmpty())
    {        
        qDebug() << "Reply is null or empty";

        emit sceneDownloadFinished(scene, false, tr("При скачивании сцены %0 получен пустой ответ").arg(scene->sceneid));
        return;
    }

    qDebug() << "SceneBand " << _downloadPathIndex;
        
    QString path = _isClip ? Storage::sceneBandClipPath(scene, reply->url().fileName(), _clipNumber) : Storage::sceneBandPath(scene, reply->url().fileName());

    QFile localFile(path);
    if (!localFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file " << qPrintable(path);

        emit sceneDownloadFinished(scene, false, tr("Не удается открыть файл для записи канала сцены %0 %1").arg(scene->sceneid).arg(path));
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

void DownloadManager::processUsgsLoginReply(const ScenePtr& scene, QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();

        emit usgsDownloadFinished(scene, false, tr("При получении сцены %0 произошла ошибка %1 %2").arg(scene->sceneid).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    //QNetworkRequest request(QString::fromUtf8("http://earthexplorer.usgs.gov/download/1854/%0/L1T/EE").arg(scene->sceneid));
    QNetworkRequest request(QString::fromUtf8("http://earthexplorer.usgs.gov/download/1854/%0/GRB/EE").arg(scene->sceneid));

    request.setAttribute(QNetworkRequest::User, QString("UsgsFirst"));

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    _networkManager.get(request);
}

void DownloadManager::processUsgsFirstReply(const ScenePtr& scene, QNetworkReply* reply)
{
    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();
        
        emit usgsDownloadFinished(scene, false, tr("При получении сцены %0 произошла ошибка %1 %2").arg(scene->sceneid).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 302)
    {
        qDebug() << "Wrong status code " << statusCode;

        emit usgsDownloadFinished(scene, true, tr("Невозможно получить сцену %0 с сервера USGS: неверный код ответа %1").arg(scene->sceneid).arg(statusCode));
        return;
    }

    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectUrl.isEmpty() || redirectUrl == _oldRedirectUrl)
    {
        qDebug() << "Wrong url " << redirectUrl;

        emit usgsDownloadFinished(scene, true, tr("Невозможно получить сцену %0 с сервера USGS: ошибка перенаправления").arg(scene->sceneid));
        return;
    }

    qDebug() << "Redirect " << redirectUrl;

    _oldRedirectUrl = redirectUrl;    

    QNetworkRequest request(redirectUrl);

    request.setAttribute(QNetworkRequest::User, QString("UsgsRedirect"));

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    _longDownloadReply = _networkManager.get(request);

    connect(_longDownloadReply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));
    connect(_longDownloadReply, SIGNAL(readyRead()), this, SLOT(readDataChunk()));

    //----------------------------------------
        
    _tempFile = new QFile(Storage::tempPath("tempfilename.zip"));
    if (!_tempFile->open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file ";

        emit usgsDownloadFinished(scene, true, tr("Невозможно создать файл для записи сцены %0 с сервера USGS").arg(scene->sceneid));
        return;
    }

    //----------------------------------------

    if (!_progressDialog)
    {
        _progressDialog = new QProgressDialog(qApp->activeWindow());
        _progressDialog->setMinimum(0);
        _progressDialog->setMaximum(100);

        connect(this, SIGNAL(progressChanged(int)), _progressDialog, SLOT(setValue(int)));
        connect(_progressDialog, SIGNAL(canceled()), this, SLOT(cancelDownload()));
    }

    _progressDialog->setLabelText(tr("Скачивание сцены %0 с сервера USGS").arg(scene->sceneid));
    _progressDialog->reset();
    _progressDialog->show();
}

void DownloadManager::processUsgsRedirectReply(const ScenePtr& scene, QNetworkReply* reply)
{
    _progressDialog->reset();
    _tempFile->close();
    _longDownloadReply = 0;

    if (reply->error() == QNetworkReply::OperationCanceledError)
    {
        qDebug() << "Cancelled";

        delete _tempFile;
        _tempFile = 0;

        emit usgsDownloadFinished(scene, false, tr("Получение сцены %0 отменено пользователем").arg(scene->sceneid));
        return;
    }
    else if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();

        delete _tempFile;
        _tempFile = 0;

        emit usgsDownloadFinished(scene, false, tr("При получении сцены %0 произошла ошибка %1 %2").arg(scene->sceneid).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    QString filepath = Storage::tempPath(reply->url().fileName() + ".zip");
    
    _tempFile->rename(filepath);

    delete _tempFile;
    _tempFile = 0;

    //-----------------------------------

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"file\";filename=\"%0\"").arg(reply->url().fileName() + ".zip"));

    QFile* file = new QFile(filepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(imagePart);

    QNetworkRequest request(QString("http://virtualglobe.ru/geoportalapi/scene/%0").arg(scene->sceneid));

    request.setAttribute(QNetworkRequest::User, QString("Upload"));

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    QNetworkReply* uploadReply = _networkManager.post(request, multiPart);

    multiPart->setParent(uploadReply); // delete the multiPart with the reply

    connect(uploadReply, SIGNAL(uploadProgress(qint64, qint64)), this, SLOT(onDownloadProgress(qint64, qint64)));

    //-----------------------------------

    _progressDialog->setLabelText(tr("Загрузка сцены %0 на сервер").arg(scene->sceneid));
    _progressDialog->show();
}

void DownloadManager::processUploadReply(const ScenePtr& scene, QNetworkReply* reply)
{
    _progressDialog->reset();

    if (reply->error() == QNetworkReply::OperationCanceledError)
    {
        qDebug() << "Cancelled";
        
        emit usgsDownloadFinished(scene, false, tr("Загрузка сцены %0 на сервер отменена пользователем").arg(scene->sceneid));
        return;
    }
    else if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();
        
        emit usgsDownloadFinished(scene, false, tr("При загрузке сцены %0 на сервер произошла ошибка %1 %2").arg(scene->sceneid).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    emit usgsDownloadFinished(scene, true, tr("Сцена %0 успешно получена с сервера USGS и загружена на наш сервер").arg(scene->sceneid));
}

void DownloadManager::downloadNextSceneBand(const ScenePtr& scene)
{
    assert(_downloadPathIndex < _downloadPaths.size());

    emit progressChanged(0);

    QNetworkRequest request(_downloadPaths[_downloadPathIndex]);

    QString path = _isClip ? Storage::sceneBandClipPath(scene, request.url().fileName(), _clipNumber) : Storage::sceneBandPath(scene, request.url().fileName());
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

void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if (bytesTotal != 0)
    {
        emit progressChanged(100 * bytesReceived / bytesTotal);
    }
}

void DownloadManager::cancelDownload()
{        
    //Добавить отмену аплоада

    if (_longDownloadReply)
    {
        _longDownloadReply->abort();
        _longDownloadReply = 0;
    }
    else
    {
        qDebug() << "Reply is null";
    }
}

void DownloadManager::readDataChunk()
{
    if (_tempFile && _longDownloadReply)
    {
        _tempFile->write(_longDownloadReply->readAll());
    }
    else if (!_tempFile)
    {
        qDebug() << "File is null";
    }
    else
    {
        qDebug() << "Reply is null";
    }
}