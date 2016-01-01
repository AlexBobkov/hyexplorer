#include "Storage.hpp"

#include <QSettings>
#include <QDir>
#include <QDebug>

using namespace portal;

QString Storage::overviewPath(const ScenePtr& scene, const QString& filename)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName = QString("%0/overviews/").arg(scene->sensor);    
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
    QString folderName = QString("hyperion/scenes/%0/").arg(scene->sceneId);
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

QString Storage::sceneBandDir(const ScenePtr& scene)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    return dataDir.filePath(QString("hyperion/scenes/%0/").arg(scene->sceneId));
}

QString Storage::sceneBandClipPath(const ScenePtr& scene, const QString& filename, const QString& clipName)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName = QString("hyperion/clips/%0/clip%1/").arg(scene->sceneId).arg(clipName);
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

QString Storage::sceneBandClipDir(const ScenePtr& scene, const QString& clipName)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    return dataDir.filePath(QString("hyperion/clips/%0/clip%1/").arg(scene->sceneId).arg(clipName));
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