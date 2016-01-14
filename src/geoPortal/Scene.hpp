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