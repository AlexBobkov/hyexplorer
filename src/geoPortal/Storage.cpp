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

QString Storage::sceneBandPath(const ScenePtr& scene, int band, const ClipInfoPtr& clipInfo)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();
    
    QDir dataDir(dataPath);
    QString folderName;
    QString filename;
    if (clipInfo)
    {        
        folderName = QString("Hyperion/scenes/%0/clips/clip%1/").arg(scene->sceneId).arg(clipInfo->uniqueName());

        filename = QString("%0B%1_L1T_clip.TIF").arg(scene->sceneId.mid(0, 23)).arg(band, 3, 10, QChar('0'));
    }
    else
    {
        folderName = QString("Hyperion/scenes/%0/original/").arg(scene->sceneId);

        filename = QString("%0B%1_L1T.TIF").arg(scene->sceneId.mid(0, 23)).arg(band, 3, 10, QChar('0'));
    }

    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

QString Storage::sceneBandPath(const ScenePtr& scene, const QString& filename, const ClipInfoPtr& clipInfo)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName;
    if (clipInfo)
    {
        folderName = QString("Hyperion/scenes/%0/clips/clip%1/").arg(scene->sceneId).arg(clipInfo->uniqueName());
    }
    else
    {
        folderName = QString("Hyperion/scenes/%0/original/").arg(scene->sceneId);
    }

    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

QString Storage::sceneBandDir(const ScenePtr& scene, const ClipInfoPtr& clipInfo)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    if (clipInfo)
    {
        return dataDir.filePath(QString("Hyperion/scenes/%0/clips/clip%1/").arg(scene->sceneId).arg(clipInfo->uniqueName()));
    }
    else
    {
        return dataDir.filePath(QString("Hyperion/scenes/%0/original/").arg(scene->sceneId));
    }
}

QString Storage::processedFilePath(const ScenePtr& scene, int band, const QString& processedId)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();
    
    QDir dataDir(dataPath);
    QString folderName = QString("Hyperion/scenes/%0/processed/").arg(scene->sceneId);
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