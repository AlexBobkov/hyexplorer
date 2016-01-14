#include "Operations.hpp"
#include "Storage.hpp"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QApplication>

using namespace portal;

DownloadSceneOperation::DownloadSceneOperation(const ScenePtr& scene, const ClipInfoPtr& clipInfo, QNetworkAccessManager* manager, QObject* parent) :
QObject(parent),
_networkManager(manager),
_scene(scene),
_clipInfo(clipInfo)
{
    QUrl url;
    if (!_clipInfo->isFullSize())
    {
        qDebug() << "Download clip name " << _clipInfo->uniqueName();

        osgEarth::Bounds b = *_clipInfo->bounds();

        url = QString("http://virtualglobe.ru/geoportalapi/sceneclip/%0/%1/%2?leftgeo=%3&upgeo=%4&rightgeo=%5&downgeo=%6")
            .arg(_scene->sceneId())
            .arg(_clipInfo->minBand())
            .arg(_clipInfo->maxBand())
            .arg(b.xMin(), 0, 'f', 10)
            .arg(b.yMax(), 0, 'f', 10)
            .arg(b.xMax(), 0, 'f', 10)
            .arg(b.yMin(), 0, 'f', 10);
    }
    else
    {
        qDebug() << "Download full size";

        url = QString("http://virtualglobe.ru/geoportalapi/scene/%0/%1/%2")
            .arg(_scene->sceneId())
            .arg(_clipInfo->minBand())
            .arg(_clipInfo->maxBand());
    }

    QNetworkReply* reply = _networkManager->get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            emit error(tr("При скачивании сцены %0 произошла ошибка %1 %2").arg(_scene->sceneId()).arg(reply->error()).arg(reply->errorString()));
            return;
        }

        QByteArray data = reply->readAll();
        if (data.isNull() || data.isEmpty())
        {
            emit error(tr("При скачивании сцены %0 получен пустой ответ").arg(_scene->sceneId()));
            return;
        }

        if (!data.startsWith("SUCCESS"))
        {
            emit error(tr("Сцена %0 не найдена на сервере").arg(_scene->sceneId()));
            return;
        }

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
            emit error(tr("Каналы для сцены %0 не найдены").arg(_scene->sceneId()));
            return;
        }

        _downloadPathIndex = 0;
        downloadNextSceneBand();
    });
}

DownloadSceneOperation::~DownloadSceneOperation()
{

}

void DownloadSceneOperation::downloadNextSceneBand()
{
    assert(_downloadPathIndex < _downloadPaths.size());

    emit progressChanged(0);

    QNetworkRequest request(_downloadPaths[_downloadPathIndex]);

    QString path = Storage::sceneBandPath(_scene, request.url().fileName(), _clipInfo);
    if (QFile::exists(path))
    {
        _downloadPathIndex++;
        if (_downloadPathIndex >= _downloadPaths.size())
        {
            emit finished(_scene, _clipInfo);
            return;
        }

        downloadNextSceneBand();
    }
    else
    {
        QNetworkReply* reply = _networkManager->get(request);
        connect(reply, &QNetworkReply::finished, this, [reply, this]()
        {
            reply->deleteLater();

            if (reply->error() != QNetworkReply::NoError)
            {
                emit error(tr("При скачивании сцены %0 произошла ошибка %1 %2").arg(_scene->sceneId()).arg(reply->error()).arg(reply->errorString()));
                return;
            }

            QByteArray data = reply->readAll();
            if (data.isNull() || data.isEmpty())
            {
                emit error(tr("При скачивании сцены %0 получен пустой ответ").arg(_scene->sceneId()));
                return;
            }

            qDebug() << "SceneBand " << _downloadPathIndex;

            QString path = Storage::sceneBandPath(_scene, reply->url().fileName(), _clipInfo);

            QFile localFile(path);
            localFile.open(QIODevice::WriteOnly);
            localFile.write(data);
            localFile.close();

            _downloadPathIndex++;
            if (_downloadPathIndex >= _downloadPaths.size())
            {
                emit finished(_scene, _clipInfo);
                return;
            }

            downloadNextSceneBand();
        });
    }
}

//====================================================================================

ImportSceneOperation::ImportSceneOperation(const ScenePtr& scene, QNetworkAccessManager* manager, QObject* parent) :
QObject(parent),
_networkManager(manager),
_scene(scene),
_tempFile(0),
_downloadReply(0),
_uploadReply(0),
_progressDialog(0)
{
    assert(!_scene->hasScene());

    QNetworkRequest request(QString("https://ers.cr.usgs.gov/login/"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QString options = "username=AlexBobkov&password=1qaz2wsx";

    QNetworkReply* reply = _networkManager->post(request, options.toLocal8Bit());
    connect(reply, SIGNAL(sslErrors(QList<QSslError>)), reply, SLOT(ignoreSslErrors()));
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            emit error(tr("При получении сцены %0 произошла ошибка %1 %2").arg(_scene->sceneId()).arg(reply->error()).arg(reply->errorString()));
            return;
        }

        QNetworkRequest request(QString("http://earthexplorer.usgs.gov/download/1854/%0/L1T/EE").arg(_scene->sceneId()));
        //QNetworkRequest request(QString("http://earthexplorer.usgs.gov/download/1854/%0/GRB/EE").arg(_scene->sceneId()));

        QNetworkReply* redirectReply = _networkManager->get(request);
        connect(redirectReply, &QNetworkReply::finished, this, &ImportSceneOperation::downloadScene);
    });
}

ImportSceneOperation::~ImportSceneOperation()
{
}

void ImportSceneOperation::downloadScene()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    assert(reply);

    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        emit error(tr("При получении сцены %0 произошла ошибка %1 %2").arg(_scene->sceneId()).arg(reply->error()).arg(reply->errorString()));
        return;
    }

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    if (statusCode != 302)
    {
        emit error(tr("Невозможно получить сцену %0 с сервера USGS: неверный код ответа %1").arg(_scene->sceneId()).arg(statusCode));
        return;
    }

    QUrl redirectUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirectUrl.isEmpty())
    {
        emit error(tr("Невозможно получить сцену %0 с сервера USGS: ошибка перенаправления").arg(_scene->sceneId()));
        return;
    }

    qDebug() << "Redirect " << redirectUrl;

    //----------------------------------------

    _tempFile = new QFile(Storage::tempPath("tempfilename.zip"), this);
    if (!_tempFile->open(QIODevice::WriteOnly))
    {
        emit error(tr("Невозможно создать файл для записи сцены %0 с сервера USGS").arg(_scene->sceneId()));
        return;
    }

    //----------------------------------------

    _downloadReply = _networkManager->get(QNetworkRequest(redirectUrl));

    connect(_downloadReply, &QNetworkReply::downloadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
    {
        if (bytesTotal != 0)
        {
            emit progressChanged(100 * bytesReceived / bytesTotal);
        }
    });

    connect(_downloadReply, &QNetworkReply::readyRead, this, [this]()
    {
        _tempFile->write(_downloadReply->readAll());
    });

    connect(_downloadReply, &QNetworkReply::finished, this, &ImportSceneOperation::uploadScene);

    //----------------------------------------

    _progressDialog = new QProgressDialog(qApp->activeWindow());
    _progressDialog->setMinimum(0);
    _progressDialog->setMaximum(100);
    _progressDialog->setLabelText(tr("Скачивание сцены %0 с внешнего сервера").arg(_scene->sceneId()));
    _progressDialog->reset();
    _progressDialog->show();

    connect(this, SIGNAL(progressChanged(int)), _progressDialog, SLOT(setValue(int)));
    connect(_progressDialog, &QProgressDialog::canceled, this, [this]()
    {
        if (_downloadReply)
        {
            _downloadReply->abort();
        }
        if (_uploadReply)
        {
            _uploadReply->abort();
        }
    });
}

void ImportSceneOperation::uploadScene()
{
    _progressDialog->reset();
    _downloadReply->deleteLater();
    _tempFile->close();

    if (_downloadReply->error() == QNetworkReply::OperationCanceledError)
    {
        emit error(tr("Получение сцены %0 отменено пользователем").arg(_scene->sceneId()));
        return;
    }
    else if (_downloadReply->error() != QNetworkReply::NoError)
    {
        emit error(tr("При получении сцены %0 произошла ошибка %1 %2").arg(_scene->sceneId()).arg(_downloadReply->error()).arg(_downloadReply->errorString()));
        return;
    }

    QString filepath = Storage::tempPath(_downloadReply->url().fileName());

    _tempFile->rename(filepath);
    _tempFile = 0;

    //-----------------------------------

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"file\";filename=\"%0\"").arg(_downloadReply->url().fileName()));

    QFile* file = new QFile(filepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(imagePart);

    //-----------------------------------

    _downloadReply = 0;

    QUrl url = QString("http://virtualglobe.ru/geoportalapi/scene/%0").arg(_scene->sceneId());

    _uploadReply = _networkManager->post(QNetworkRequest(url), multiPart);

    connect(_uploadReply, &QNetworkReply::uploadProgress, this, [this](qint64 bytesReceived, qint64 bytesTotal)
    {
        if (bytesTotal != 0)
        {
            emit progressChanged(100 * bytesReceived / bytesTotal);
        }
    });

    connect(_uploadReply, &QNetworkReply::finished, this, [this]()
    {
        _uploadReply->deleteLater();

        _progressDialog->reset();
        _progressDialog->deleteLater();

        if (_uploadReply->error() == QNetworkReply::OperationCanceledError)
        {
            emit error(tr("Загрузка сцены %0 на сервер отменена пользователем").arg(_scene->sceneId()));
            return;
        }
        else if (_uploadReply->error() != QNetworkReply::NoError)
        {
            emit error(tr("При загрузке сцены %0 на сервер произошла ошибка %1 %2").arg(_scene->sceneId()).arg(_uploadReply->error()).arg(_uploadReply->errorString()));
            return;
        }

        emit finished(_scene);
    });

    //-----------------------------------

    _progressDialog->setLabelText(tr("Загрузка сцены %0 на сервер").arg(_scene->sceneId()));
    _progressDialog->show();
}

//====================================================================================

ProcessingOperation::ProcessingOperation(const ScenePtr& scene, int band, const QString& toolFilepath, const QString& inputFilepath, const QString& outputFilepath, QNetworkAccessManager* manager, QObject* parent) :
QObject(parent),
_networkManager(manager),
_scene(scene),
_band(band),
_toolFilepath(toolFilepath),
_inputFilepath(inputFilepath),
_outputFilepath(outputFilepath),
_process(0)
{
    QString workingDirectory = QFileInfo(_toolFilepath).absolutePath();

    {
        QFile data(workingDirectory + "/data.txt");
        data.open(QFile::WriteOnly);

        QTextStream out(&data);
        out << _inputFilepath.toLocal8Bit() << "\n";

        data.close();
    }

    //------------------------------------

    {
        QFile result(workingDirectory + "/result.txt");
        result.open(QFile::WriteOnly);

        QTextStream out(&result);
        out << _outputFilepath.toLocal8Bit() << "\n";

        result.close();
    }

    //------------------------------------

    qDebug() << "Image correction started";

    _process = new QProcess(this);
    _process->setWorkingDirectory(workingDirectory);

    connect(_process, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, [this](QProcess::ProcessError errorCode)
    {
        qDebug() << "Image correnction failed. Error code" << errorCode;

        _process->deleteLater();
        _process->setParent(0);

        emit error(tr("Ошибка при выполнении обработки. Код %0").arg(errorCode));
    });

    connect(_process, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [this](int exitCode, QProcess::ExitStatus exitStatus)
    {
        qDebug() << "Image correnction finished. Exit code" << exitCode << "exit status" << exitStatus;

        _process->deleteLater();
        _process->setParent(0);

        if (!QFileInfo::exists(_outputFilepath))
        {
            emit error(tr("Обработка прервана пользователем"));
            return;
        }

        uploadProccessedFile();
    });

    _process->start(_toolFilepath, QStringList());

    qDebug() << "Start " << QFileInfo(_toolFilepath).fileName();
    qDebug() << "Working dir " << workingDirectory;
}

ProcessingOperation::~ProcessingOperation()
{

}

void ProcessingOperation::uploadProccessedFile()
{
    qDebug() << "Upload " << _outputFilepath;

    QFileInfo outputFileInfo(_outputFilepath);

    //------------------------------------

    QString paramsFilepath = outputFileInfo.absolutePath() + "/" + outputFileInfo.completeBaseName() + ".txt";
    if (!QFileInfo::exists(paramsFilepath))
    {
        emit error(tr("Не найден файл с параметрами обработки %0").arg(paramsFilepath));
        return;
    }

    QFile paramsFile(paramsFilepath);
    paramsFile.open(QIODevice::ReadOnly | QIODevice::Text);

    QTextStream in(&paramsFile);
    QString line1 = in.readLine();
    QString line2 = in.readLine();

    if (line2.isNull())
    {
        emit error(tr("Не удалось прочитать параметры обработки %0").arg(paramsFilepath));
        return;
    }

    paramsFile.close();

    //------------------------------------

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType, this);

    //------------------------------------

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"file\";filename=\"%0\"").arg(outputFileInfo.fileName()));

    QFile* file = new QFile(_outputFilepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(imagePart);

    //------------------------------------

    QHttpPart bandPart;
    bandPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"band\""));
    bandPart.setBody(QByteArray::number(_band));

    multiPart->append(bandPart);

    //------------------------------------

    QHttpPart appnamePart;
    appnamePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"appname\""));
    appnamePart.setBody(QFileInfo(_toolFilepath).fileName().toUtf8());

    multiPart->append(appnamePart);

    //------------------------------------

    QHttpPart processingtimePart;
    processingtimePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"processingtime\""));
    processingtimePart.setBody(QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8());

    multiPart->append(processingtimePart);

    //------------------------------------

    QHttpPart paramsPart;
    paramsPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"params\""));
    paramsPart.setBody(line2.toUtf8());

    multiPart->append(paramsPart);

    //------------------------------------

    QUrl url = QString("http://virtualglobe.ru/geoportalapi/processed/%0").arg(_scene->sceneId());

    QNetworkReply* reply = _networkManager->post(QNetworkRequest(url), multiPart);
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        reply->deleteLater();

        if (reply->error() != QNetworkReply::NoError)
        {
            emit error(tr("Ошибка при загрузке файла на сервер %1 %2").arg(reply->error()).arg(reply->errorString()));
            return;
        }

        emit finished();
    });
}
