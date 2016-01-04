#pragma once

#include <osgEarth/Bounds>

#include <QString>
#include <QMetaType>

#include <memory>

namespace portal
{
    QString genetrateRandomName();

    class ClipInfo
    {
    public:
        ClipInfo(const osgEarth::Bounds& bounds);

        bool valid() const { return _minBand > 0 && _maxBand > _minBand; }

        const osgEarth::Bounds& bounds() const { return _bounds; }

        int minBand() const { return _minBand; }
        void setMinBand(int b) { _minBand = b; }

        int maxBand() const { return _maxBand; }
        void setMaxBand(int b) { _maxBand = b; }

        QString uniqueName() const { return _name; }

    protected:
        osgEarth::Bounds _bounds;

        int _minBand;
        int _maxBand;

        QString _name;
    };

    typedef std::shared_ptr<ClipInfo> ClipInfoPtr;
}

Q_DECLARE_METATYPE(portal::ClipInfoPtr);