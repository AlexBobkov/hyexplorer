#pragma once

#include "Scene.hpp"

#include <QString>

namespace portal
{
    class Storage
    {
    public:
        static QString overviewPath(const QString& filename);
        static QString sceneBandPath(const ScenePtr& scene, const QString& filename);
        static QString sceneBandClipPath(const ScenePtr& scene, const QString& filename, int clipNumber);
        static QString tempPath(const QString& filename);
    };
}
