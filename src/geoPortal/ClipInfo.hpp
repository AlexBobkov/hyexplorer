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