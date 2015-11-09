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

void Downloader::process(const std::queue<QString>& queue)
{
    _queue = queue;

    if (!_queue.empty())
    {
        QString sceneid = _queue.front();
        _queue.pop();
        processScene(sceneid);
    }
}

void Downloader::processScene(const QString& sceneid)
{
    QNetworkRequest request(QString::fromUtf8("http://earthexplorer.usgs.gov/metadata/1854/%0/").arg(sceneid));
    request.setAttribute(QNetworkRequest::User, QString("Metadata"));
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), sceneid);    
    _networkManager.get(request);
}

void Downloader::processMetadata(const QString& sceneid, const QByteArray& data)
{
    int startIndex = data.indexOf("http://earthexplorer.usgs.gov/browse/eo-1/hyp");
    if (startIndex != -1)
    {
        int endIndex = data.indexOf(".jpeg", startIndex);
        if (endIndex != -1)
        {
            QByteArray overviewUrlName = data.mid(startIndex, endIndex - startIndex + 5);
            QUrl url(overviewUrlName.constData());

            QNetworkRequest request(url);
            request.setAttribute(QNetworkRequest::User, QString("Download"));
            request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), sceneid);
            _networkManager.get(request);

            return;
        }
    }

    qDebug() << "Failed to process metadata for scene " << sceneid;
}

void Downloader::processOverview(const QString& sceneid, const QString& filename, const QByteArray& data)
{
    QString filepath = _dataDir.filePath(downdloadFolder + QString("/") + filename);

    QFile localFile(filepath);
    if (!localFile.open(QIODevice::WriteOnly))
    {
        qDebug() << "Failed to open file " << qPrintable(filepath);
        return;
    }

    localFile.write(data);
    localFile.close();

    uploadOverview(sceneid, filepath);
}

void Downloader::uploadOverview(const QString& sceneid, const QString& filepath)
{
    QFileInfo info(filepath);
    if (!info.exists())
    {
        qDebug() << "File does not exit " << filepath;
        return;
    }

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"%0\";filename=\"%0\"").arg(info.fileName()));

    QFile* file = new QFile(filepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
        
    multiPart->append(imagePart);

    QNetworkRequest request(QString("http://localhost:5000/overview/%0").arg(sceneid));
    request.setAttribute(QNetworkRequest::User, QString("Upload"));
    QNetworkReply* reply = _networkManager.post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
}

void Downloader::onReplyReceived(QNetworkReply* reply)
{
    qDebug() << "Request is completed " << reply->url().toString();

    QByteArray data = reply->readAll();
    if (!data.isNull() && !data.isEmpty())
    {
        QString requestType = reply->request().attribute(QNetworkRequest::User).toString();
        QString sceneid = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).toString();

        if (requestType == "Metadata")
        {
            processMetadata(sceneid, data);
        }
        else if (requestType == "Download")
        {
            processOverview(sceneid, reply->url().fileName(), data);
        }
        else if (requestType == "Upload")
        {
            if (!_queue.empty())
            {
                QString sceneid = _queue.front();
                _queue.pop();
                processScene(sceneid);
            }
            else
            {
                qDebug() << "Finish!";
            }
        }
    }

    reply->deleteLater();
}
