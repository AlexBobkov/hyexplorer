#pragma once

#include <osg/Vec3d>

#include <osgEarthFeatures/Feature>

#include <QDateTime>
#include <QString>
#include <QMetaType>

#include <boost/optional.hpp>

#include <memory>

namespace portal
{
    struct Scene
    {
        std::size_t id;

        QString sensor;
        QString sceneid;

        QDateTime sceneTime;

        boost::optional<int> cloundMin;
        boost::optional<int> cloundMax;

        boost::optional<int> orbitPath;
        boost::optional<int> orbitRow;

        boost::optional<int> targetPath;
        boost::optional<int> targetRow;

        boost::optional<QString> processingLevel;

        boost::optional<double> sunAzimuth;
        boost::optional<double> sunElevation;
        boost::optional<double> inclination;
        boost::optional<double> lookAngle;

        osg::ref_ptr<osgEarth::Features::Feature> feature;

        osg::Vec3d swCorner;
        osg::Vec3d seCorner;
        osg::Vec3d neCorner;
        osg::Vec3d nwCorner;

        bool hasOverview;
        bool hasScene;

        boost::optional<QString> overviewName;
    };

    typedef std::shared_ptr<Scene> ScenePtr;    
}

Q_DECLARE_METATYPE(portal::ScenePtr);