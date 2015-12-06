#include "Storage.hpp"

#include <QSettings>
#include <QDir>

using namespace portal;

QString Storage::overviewPath(const QString& filename)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName("hyperion/overviews/");
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

QString Storage::sceneBandPath(const ScenePtr& scene, const QString& filename)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName = QString("hyperion/scenes/%0/").arg(scene->sceneid);
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

QString Storage::sceneBandClipPath(const ScenePtr& scene, const QString& filename, int clipNumber)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName = QString("hyperion/clips/%0/clip%1/").arg(scene->sceneid).arg(clipNumber);
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

QString Storage::tempPath(const QString& filename)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName("hyperion/temp/");
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}