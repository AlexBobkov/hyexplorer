#pragma once

#include "Scene.hpp"

#include <osgEarth/Bounds>

#include <QString>
#include <QMetaType>

#include <memory>

namespace portal
{
    class ClipInfo
    {
    public:
        ClipInfo(const ScenePtr& scene);
        ClipInfo(const ScenePtr& scene, const osgEarth::Bounds& bounds);

        ScenePtr scene() const { return _scene; }

        bool valid() const { return _minBand > 0 && _maxBand > _minBand; }
        bool isFullSize() const { return !_bounds.is_initialized(); }

        int minBand() const { return _minBand; }
        void setMinBand(int b) { _minBand = b; }

        int maxBand() const { return _maxBand; }
        void setMaxBand(int b) { _maxBand = b; }

        QString uniqueName() const { return _uniqueName; }

        const boost::optional<osgEarth::Bounds>& bounds() const { return _bounds; }

    protected:
        ScenePtr _scene;

        int _minBand;
        int _maxBand;

        boost::optional<osgEarth::Bounds> _bounds;
        QString _uniqueName;
    };

    typedef std::shared_ptr<ClipInfo> ClipInfoPtr;
}

Q_DECLARE_METATYPE(portal::ClipInfoPtr);