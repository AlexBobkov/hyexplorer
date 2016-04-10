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

#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDir>

#include <queue>

class Downloader : public QObject
{
    Q_OBJECT

public:
    explicit Downloader(QObject* parent = 0);
    virtual ~Downloader();

    void setQueue(const std::queue<QString>& queue);

    void processScene(const QString& sceneid);
    void processMetadata(const QString& sceneid, const QByteArray& data);
    void processOverview(const QString& sceneid, const QString& filename, const QByteArray& data);
    void uploadOverview(const QString& sceneid, const QString& filepath);

private slots:
    void onReplyReceived(QNetworkReply* reply);

private:
    void startNextScene();

    QNetworkAccessManager _networkManager;
    QDir _dataDir;

    std::queue<QString> _queue;
};
