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
    QString folderName = QString("Hyperion/scenes/%0/").arg(scene->sceneId);
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
    return dataDir.filePath(QString("Hyperion/scenes/%0/").arg(scene->sceneId));
}

QString Storage::sceneBandClipPath(const ScenePtr& scene, const QString& filename, const QString& clipName)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName = QString("Hyperion/clips/%0/clip%1/").arg(scene->sceneId).arg(clipName);
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
    return dataDir.filePath(QString("Hyperion/clips/%0/clip%1/").arg(scene->sceneId).arg(clipName));
}

QString Storage::processedFilePath(const ScenePtr& scene, int band, const QString& processedId)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();
    
    QDir dataDir(dataPath);
    QString folderName("Hyperion/processed/");
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    QString outputFilename = QString("%0B%1_L1T_%2.TIF").arg(scene->sceneId.mid(0, 23)).arg(band, 3, 10, QChar('0')).arg(processedId);

    return dataDir.filePath(folderName + outputFilename);
}

QString Storage::tempPath(const QString& filename)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName("Hyperion/temp/");
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}