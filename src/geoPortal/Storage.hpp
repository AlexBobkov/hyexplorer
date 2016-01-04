﻿#pragma once

#include "Scene.hpp"

#include <QString>

namespace portal
{
    class Storage
    {
    public:
        static QString overviewPath(const ScenePtr& scene, const QString& filename);

        static QString sceneBandPath(const ScenePtr& scene, const QString& filename);
        static QString sceneBandDir(const ScenePtr& scene);

        static QString sceneBandClipPath(const ScenePtr& scene, const QString& filename, const QString& clipName);
        static QString sceneBandClipDir(const ScenePtr& scene, const QString& clipName);

        static QString processedFilePath(const ScenePtr& scene, int band, const QString& processedId);
        
        static QString tempPath(const QString& filename);
    };
}
