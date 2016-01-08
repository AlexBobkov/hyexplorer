#pragma once

#include "Scene.hpp"
#include "ClipInfo.hpp"

#include <QString>
#include <QDir>

namespace portal
{
    class Storage
    {
    public:
        static QString overviewPath(const ScenePtr& scene, const QString& filename);

        static QString sceneBandPath(const ScenePtr& scene, int band, const ClipInfoPtr& clipInfo = ClipInfoPtr());
        static QString sceneBandPath(const ScenePtr& scene, const QString& filename, const ClipInfoPtr& clipInfo = ClipInfoPtr());
        static QDir sceneBandDir(const ScenePtr& scene, const ClipInfoPtr& clipInfo = ClipInfoPtr());

        static QString processedFilePath(const ScenePtr& scene, int band, const QString& processedId);
        static QDir processedFileDir(const ScenePtr& scene);
        
        static QString tempPath(const QString& filename);
    };
}
