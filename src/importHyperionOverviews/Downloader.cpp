#include "Downloader.hpp"

#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QSettings>
#include <QHttpMultiPart>

const QString downdloadFolder = "temp";

Downloader::Downloader(QObject* parent) :
QObject(parent)
{
    connect(&_networkManager, SIGNAL(finished(QNetworkReply*)), SLOT(onReplyReceived(QNetworkReply*)));

    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();
    _dataDir = QDir(dataPath);
    if (!_dataDir.exists(downdloadFolder))
    {
        _dataDir.mkpath(downdloadFolder);
    }
}

Downloader::~Downloader()
{
}

void Downloader::downloadFile(const QString& url)
{
    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, QString("Download"));
    _networkManager.get(request);
}

void Downloader::uploadFile(const QString& url, const QString& filepath)
{
    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"%0\";filename=\"%0\"").arg(filepath));

    QFile* file = new QFile(filepath);
    if (!file->open(QIODevice::ReadOnly))
    {
        qDebug() << "Failed to read file " << filepath;
        delete file;
        return;
    }

    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
        
    multiPart->append(imagePart);

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::User, QString("Upload"));
    QNetworkReply* reply = _networkManager.post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
}

void Downloader::onReplyReceived(QNetworkReply* reply)
{
    qDebug() << "File is downloaded " << reply->url().toString();

    QByteArray data = reply->readAll();
    if (!data.isNull() && !data.isEmpty())
    {
        QString requestType = reply->request().attribute(QNetworkRequest::User).toString();

        if (requestType == "Download")
        {
            QString overviewFilepath = _dataDir.filePath(downdloadFolder + QString("/") + reply->url().fileName());

            QFile localFile(overviewFilepath);
            if (!localFile.open(QIODevice::WriteOnly))
            {
                qDebug() << "Failed to open file " << qPrintable(overviewFilepath);
                return;
            }

            localFile.write(data);
            localFile.close();
        }
        else if (requestType == "Upload")
        {
            qDebug() << "Uploaded finished" << data;
        }
    }

    reply->deleteLater();

    qApp->quit();
}
