/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "Downloader.hpp"

#include <QFile>
#include <QDebug>
#include <QApplication>
#include <QSettings>
#include <QHttpMultiPart>

const QString downdloadFolder = "temp_aviris";

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

void Downloader::setQueue(const std::queue<QString>& queue, const std::map<QString, QString>& overviewMap)
{
    _queue = queue;
    _overviewMap = overviewMap;

    if (!_queue.empty())
    {
        QString sceneid = _queue.front();
        _queue.pop();
        processScene(sceneid);
    }
}

void Downloader::processScene(const QString& sceneid)
{
    auto url = _overviewMap.find(sceneid);
    if (url == _overviewMap.end())
    {
        qDebug() << "Failed to find url for scene " << sceneid;
        return;
    }

    QNetworkRequest request((*url).second);
    request.setAttribute(QNetworkRequest::User, QString("Download"));
    request.setAttribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1), sceneid);    
    _networkManager.get(request);
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
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"file\";filename=\"%0\"").arg(info.fileName()));

    QFile* file = new QFile(filepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart
        
    multiPart->append(imagePart);

#if 0
    QNetworkRequest request(QString("http://localhost:5000/overview/%0").arg(sceneid));
#elif 0
    QNetworkRequest request(QString("http://178.62.140.44:5000/overview/%0").arg(sceneid));
#else
    QNetworkRequest request(QString("http://virtualglobe.ru/geoportalapi/overview/AVIRIS/%0").arg(sceneid));
#endif
    request.setAttribute(QNetworkRequest::User, QString("Upload"));
    QNetworkReply* reply = _networkManager.post(request, multiPart);
    multiPart->setParent(reply); // delete the multiPart with the reply
}

void Downloader::onReplyReceived(QNetworkReply* reply)
{
    qDebug() << "Request is completed " << reply->url().toString();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Error " << reply->error() << " " << reply->errorString();

        startNextScene();

        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    if (!data.isNull() && !data.isEmpty())
    {
        QString requestType = reply->request().attribute(QNetworkRequest::User).toString();
        QString sceneid = reply->request().attribute((QNetworkRequest::Attribute)(QNetworkRequest::User + 1)).toString();

        if (requestType == "Download")
        {
            processOverview(sceneid, reply->url().fileName(), data);
        }
        else if (requestType == "Upload")
        {
            startNextScene();
        }
    }
    else
    {
        qDebug() << "Reply is empty";
    }

    reply->deleteLater();
}

void Downloader::startNextScene()
{
    if (!_queue.empty())
    {
        qDebug() << "Remains " << _queue.size();

        QString sceneid = _queue.front();
        _queue.pop();
        processScene(sceneid);
    }
    else
    {
        qDebug() << "Finish!";
    }
}