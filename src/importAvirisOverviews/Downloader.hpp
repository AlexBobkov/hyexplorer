﻿#pragma once

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

    void setQueue(const std::queue<QString>& queue, const std::map<QString, QString>& overviewMap);

    void processScene(const QString& sceneid);
    void processOverview(const QString& sceneid, const QString& filename, const QByteArray& data);
    void uploadOverview(const QString& sceneid, const QString& filepath);

private slots:
    void onReplyReceived(QNetworkReply* reply);

private:
    void startNextScene();

    QNetworkAccessManager _networkManager;
    QDir _dataDir;

    std::queue<QString> _queue;
    std::map<QString, QString> _overviewMap;
};
