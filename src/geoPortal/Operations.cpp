#include "Operations.hpp"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QHttpMultiPart>
#include <QNetworkRequest>
#include <QNetworkReply>

using namespace portal;

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

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

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

    multiPart->setParent(reply); // delete the multiPart with the reply
}
