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

#include <osg/Vec3d>

#include <osgEarthFeatures/Feature>

#include <QDateTime>
#include <QString>
#include <QMetaType>
#include <QVariant>

#include <boost/optional.hpp>

#include <memory>

namespace portal
{
    class Scene
    {
    public:
        Scene();

        int id() const;

        QString sensor() const;
        QString sceneId() const;
                
        bool hasOverview() const;

        bool hasScene() const;
        void setSceneExistence(bool b);

        QString overviewName() const;
        QString sceneUrl() const;

        //-----------------------------

        osg::ref_ptr<osgEarth::Symbology::Geometry> geometry() const { return _geometry; }
        void setGeometry(const osg::ref_ptr<osgEarth::Symbology::Geometry>& geometry) { _geometry = geometry; }

        //-----------------------------

        QVariant attrib(const QString& key) const;
        void setAttrib(const QString& key, const QVariant& value) { _attribs[key] = value; }

        bool hasKey(const QString& key) const;

        const std::map<QString, QVariant>& attribs() const { return _attribs; }

    protected:
        osg::ref_ptr<osgEarth::Symbology::Geometry> _geometry;

        std::map<QString, QVariant> _attribs;
    };

    typedef std::shared_ptr<Scene> ScenePtr;
}

Q_DECLARE_METATYPE(portal::ScenePtr);