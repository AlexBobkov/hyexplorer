#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDir>

class Downloader : public QObject
{
    Q_OBJECT

public:
    explicit Downloader(QObject* parent = 0);
    virtual ~Downloader();

    void downloadFile(const QString& url);
    void uploadFile(const QString& url, const QString& filepath);

private slots:
    void onReplyReceived(QNetworkReply* reply);

private:
    QNetworkAccessManager _networkManager;
    QDir _dataDir;
};
