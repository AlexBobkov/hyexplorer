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

QString Storage::sceneBandClipPath(const ScenePtr& scene, const QString& filename, int clipNumber)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir dataDir(dataPath);
    QString folderName = QString("hyperion/clips/%0/clip%1/").arg(scene->sceneId).arg(clipNumber);
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName + filename);
}

int Storage::nextClipNumber(const ScenePtr& scene)
{
    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();

    QDir clipsDir(dataPath + QString("/hyperion/clips/%0/").arg(scene->sceneId));
    QStringList entries = clipsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    int clipNumber = 0;

    for (const auto& s : entries)
    {
        bool ok = false;
        int num = s.mid(4, s.size() - 4).toInt(&ok); //clip<num> pattern
        if (ok && num > clipNumber)
        {
            clipNumber = num;
        }
    }

    clipNumber++;

    return clipNumber;
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