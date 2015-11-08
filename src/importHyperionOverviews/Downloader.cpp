#include "Downloader.hpp"

#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QSettings>

const QString downdloadFolder = "temp";

Downloader::Downloader(QObject* parent) :
QObject(parent)
{
    connect(&_networkManager, SIGNAL(finished(QNetworkReply*)), SLOT(onFileDownloaded(QNetworkReply*)));

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

void Downloader::onFileDownloaded(QNetworkReply* reply)
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
    }

    qApp->quit();
}
