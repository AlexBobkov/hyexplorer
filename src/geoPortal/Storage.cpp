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

#include "Storage.hpp"

#include <QSettings>
#include <QDir>
#include <QDebug>

using namespace portal;

QString Storage::overviewPath(const ScenePtr& scene, const QString& filename)
{
    QSettings settings;
    QDir dataDir(settings.value("StoragePath").toString());

    QString folderName = QString("%0/overviews").arg(scene->sensor());
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return QDir(dataDir.filePath(folderName)).filePath(filename);
}

QString Storage::sceneBandPath(const ScenePtr& scene, int band, const ClipInfoPtr& clipInfo)
{
    QSettings settings;
    QDir dataDir(settings.value("StoragePath").toString());

    QString folderName;
    QString filename;
    if (!clipInfo->isFullSize())
    {        
        folderName = QString("Hyperion/scenes/%0/clips/clip%1").arg(scene->sceneId()).arg(clipInfo->uniqueName());
        filename = QString("%0B%1_L1T_clip.TIF").arg(scene->sceneId().mid(0, 23)).arg(band, 3, 10, QChar('0'));
    }
    else
    {
        folderName = QString("Hyperion/scenes/%0/original").arg(scene->sceneId());
        filename = QString("%0B%1_L1T.TIF").arg(scene->sceneId().mid(0, 23)).arg(band, 3, 10, QChar('0'));
    }

    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return QDir(dataDir.filePath(folderName)).filePath(filename);
}

QString Storage::sceneBandPath(const ScenePtr& scene, const QString& filename, const ClipInfoPtr& clipInfo)
{
    QSettings settings;
    QDir dataDir(settings.value("StoragePath").toString());

    QString folderName;
    if (!clipInfo->isFullSize())
    {
        folderName = QString("Hyperion/scenes/%0/clips/clip%1").arg(scene->sceneId()).arg(clipInfo->uniqueName());
    }
    else
    {
        folderName = QString("Hyperion/scenes/%0/original").arg(scene->sceneId());
    }
    
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return QDir(dataDir.filePath(folderName)).filePath(filename);
}

QDir Storage::sceneBandDir(const ScenePtr& scene, const ClipInfoPtr& clipInfo)
{
    QSettings settings;
    QDir dataDir(settings.value("StoragePath").toString());

    if (!clipInfo->isFullSize())
    {
        return dataDir.filePath(QString("Hyperion/scenes/%0/clips/clip%1").arg(scene->sceneId()).arg(clipInfo->uniqueName()));
    }
    else
    {
        return dataDir.filePath(QString("Hyperion/scenes/%0/original").arg(scene->sceneId()));
    }
}

QString Storage::processedFilePath(const ScenePtr& scene, int band, const QString& processedId)
{
    QSettings settings;
    QDir dataDir(settings.value("StoragePath").toString());

    QString folderName = QString("Hyperion/scenes/%0/processed").arg(scene->sceneId());
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    QString outputFilename = QString("%0B%1_L1T_%2.TIF").arg(scene->sceneId().mid(0, 23)).arg(band, 3, 10, QChar('0')).arg(processedId);

    return QDir(dataDir.filePath(folderName)).filePath(outputFilename);
}

QDir Storage::processedFileDir(const ScenePtr& scene)
{
    QSettings settings;    
    QDir dataDir(settings.value("StoragePath").toString());

    QString folderName = QString("Hyperion/scenes/%0/processed").arg(scene->sceneId());
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return dataDir.filePath(folderName);
}

QString Storage::tempPath(const QString& filename)
{
    QSettings settings;
    QDir dataDir(settings.value("StoragePath").toString());

    QString folderName("Hyperion/temp");
    if (!dataDir.exists(folderName))
    {
        dataDir.mkpath(folderName);
    }

    return QDir(dataDir.filePath(folderName)).filePath(filename);
}