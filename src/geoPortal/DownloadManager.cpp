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
_dataManager(dataManager),
_progressDialog(0),
_longDownloadReply(0),
_tempFile(0)
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

void DownloadManager::importScene(const ScenePtr& scene)
{
    if (_longDownloadReply)
    {
        qDebug() << "There is another scene importing";
        return;
    }

    if (scene->hasScene())
    {
        qDebug() << "Scene is already on the server";
        return;
    }

    QNetworkRequest request(QString("https://ers.cr.usgs.gov/login/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString options = "username=AlexBobkov&password=1qaz2wsx";

    QNetworkReply* reply = _dataManager->networkAccessManager().post(request, options.toLocal8Bit());
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
    connect(reply, &QNetworkReply::finished, this, [reply, this, scene]()
    {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Error " << reply->error() << " " << reply->errorString();

            emit importFinished(scene, false, tr("При получении сцены %0 произошла ошибка %1 %2").arg(scene->sceneId()).arg(reply->error()).arg(reply->errorString()));
            return;
        }

        QNetworkRequest request(QString("http://earthexplorer.usgs.gov/download/1854/%0/L1T/EE").arg(scene->sceneId()));
        //QNetworkRequest request(QString("http://earthexplorer.usgs.gov/download/1854/%0/GRB/EE").arg(scene->sceneId()));

        QVariant v;
        v.setValue(scene);
        request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

        QNetworkReply* redirectReply = _dataManager->networkAccessManager().get(request);
        connect(redirectReply, &QNetworkReply::finished, this, &DownloadManager::processRedirectReply);
    });
}

void DownloadManager::processRedirectReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    assert(reply);

    reply->deleteLater();

    ScenePtr scene = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).value<ScenePtr>();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();
        
        emit importFinished(scene, false, tr("При получении сцены %0 произошла ошибка %1 %2").arg(scene->sceneId()).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 302)
    {
        qDebug() << "Wrong status code " << statusCode;

        emit importFinished(scene, true, tr("Невозможно получить сцену %0 с сервера USGS: неверный код ответа %1").arg(scene->sceneId()).arg(statusCode));
        return;
    }

    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectUrl.isEmpty() || redirectUrl == _oldRedirectUrl)
    {
        qDebug() << "Wrong url " << redirectUrl;

        emit importFinished(scene, true, tr("Невозможно получить сцену %0 с сервера USGS: ошибка перенаправления").arg(scene->sceneId()));
        return;
    }

    qDebug() << "Redirect " << redirectUrl;

    _oldRedirectUrl = redirectUrl;

    //----------------------------------------

    _tempFile = new QFile(Storage::tempPath("tempfilename.zip"));
    if (!_tempFile->open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file ";

        emit importFinished(scene, true, tr("Невозможно создать файл для записи сцены %0 с сервера USGS").arg(scene->sceneId()));
        return;
    }

    //----------------------------------------

    QNetworkRequest request(redirectUrl);

    QVariant v;
    v.setValue(scene);
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), v);

    _longDownloadReply = _dataManager->networkAccessManager().get(request);

    _tempFile->setParent(_longDownloadReply); //delete temp file with the reply

    connect(_longDownloadReply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
    {
        if (bytesTotal != 0)
        {
            emit progressChanged(100 * bytesReceived / bytesTotal);
        }
    });

    connect(_longDownloadReply, &QNetworkReply::readyRead, this, [this]()
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
    });

    connect(_longDownloadReply, &QNetworkReply::finished, this, &DownloadManager::processDownloadReply);
    
    //----------------------------------------

    if (!_progressDialog)
    {
        _progressDialog = new QProgressDialog(qApp->activeWindow());
        _progressDialog->setMinimum(0);
        _progressDialog->setMaximum(100);

        connect(this, SIGNAL(progressChanged(int)), _progressDialog, SLOT(setValue(int)));
        connect(_progressDialog, &QProgressDialog::canceled, this, [this]()
        {
            if (_longDownloadReply)
            {
                _longDownloadReply->abort();
                _longDownloadReply = 0;
            }
            else
            {
                qDebug() << "Reply is null";
            }
        });
    }

    _progressDialog->setLabelText(tr("Скачивание сцены %0 с сервера USGS").arg(scene->sceneId()));
    _progressDialog->reset();
    _progressDialog->show();
}

void DownloadManager::processDownloadReply()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    assert(reply);
    assert(_longDownloadReply == reply);

    reply->deleteLater();

    ScenePtr scene = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).value<ScenePtr>();

    _progressDialog->reset();    
    _longDownloadReply->deleteLater();
    _longDownloadReply = 0;

    if (reply->error() == QNetworkReply::OperationCanceledError)
    {
        qDebug() << "Cancelled";

        _tempFile->close();
        _tempFile = 0;

        emit importFinished(scene, false, tr("Получение сцены %0 отменено пользователем").arg(scene->sceneId()));
        return;
    }
    else if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();

        _tempFile->close();
        _tempFile = 0;

        emit importFinished(scene, false, tr("При получении сцены %0 произошла ошибка %1 %2").arg(scene->sceneId()).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    QString filepath = Storage::tempPath(reply->url().fileName());
    
    _tempFile->close();
    _tempFile->rename(filepath);
    _tempFile = 0;

    //-----------------------------------

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"file\";filename=\"%0\"").arg(reply->url().fileName()));

    QFile* file = new QFile(filepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(imagePart);

    //-----------------------------------

    QUrl url = QString("http://virtualglobe.ru/geoportalapi/scene/%0").arg(scene->sceneId());
    
    QNetworkReply* uploadReply = _dataManager->networkAccessManager().post(QNetworkRequest(url), multiPart);

    multiPart->setParent(uploadReply); // delete the multiPart with the reply

    connect(uploadReply, &QNetworkReply::uploadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
    {
        if (bytesTotal != 0)
        {
            emit progressChanged(100 * bytesReceived / bytesTotal);
        }
    });

    connect(uploadReply, &QNetworkReply::finished, this, [uploadReply, this, scene]()
    {
        uploadReply->deleteLater();

        _progressDialog->reset();

        if (uploadReply->error() == QNetworkReply::OperationCanceledError)
        {
            qDebug() << "Cancelled";

            emit importFinished(scene, false, tr("Загрузка сцены %0 на сервер отменена пользователем").arg(scene->sceneId()));
            return;
        }
        else if (uploadReply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Error " << uploadReply->error() << " " << uploadReply->errorString();

            emit importFinished(scene, false, tr("При загрузке сцены %0 на сервер произошла ошибка %1 %2").arg(scene->sceneId()).arg(uploadReply->error()).arg(uploadReply->errorString()));
            return;
        }

        emit importFinished(scene, true, tr("Сцена %0 успешно получена с сервера USGS и загружена на наш сервер").arg(scene->sceneId()));
    });

    //-----------------------------------

    _progressDialog->setLabelText(tr("Загрузка сцены %0 на сервер").arg(scene->sceneId()));
    _progressDialog->show();
}

