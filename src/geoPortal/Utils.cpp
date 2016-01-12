#include "Utils.hpp"

#include <QFileInfo>
#include <QUrl>
#include <QDir>
#include <QDesktopServices>
#include <QProcess>

using namespace portal;

namespace
{
    std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
}

QString portal::genetrateRandomName(std::size_t size)
{
    QString name;    
    name.resize(size);

    for (int i = 0; i < name.size(); i++)
    {
        name[i] = alphabet[rand() % alphabet.size()];
    }

    return name;
}

void portal::openExplorer(const QString& path)
{
    if (QFileInfo(path).isDir())
    {
        QDesktopServices::openUrl(QUrl(QString("file:///") + path.toLocal8Bit()));
    }
    else
    {
        QProcess::startDetached(QString("explorer.exe /select,\"%0\"").arg(QDir::toNativeSeparators(path.toLocal8Bit())));
    }
}