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
        QString sceneId;

        QDateTime sceneTime;

        double pixelSize;
                
        osg::ref_ptr<osgEarth::Symbology::Geometry> geometry;
        
        boost::optional<double> sunAzimuth;
        boost::optional<double> sunElevation;

        bool hasOverview;
        bool hasScene;

        boost::optional<QString> overviewName;
        boost::optional<QString> sceneUrl;

        //-- Hyperion

        boost::optional<int> orbitPath;
        boost::optional<int> orbitRow;

        boost::optional<int> targetPath;
        boost::optional<int> targetRow;

        boost::optional<QString> processingLevel;

        boost::optional<double> inclination;
        boost::optional<double> lookAngle;

        boost::optional<int> cloundMin;
        boost::optional<int> cloundMax;

        //-- AVIRIS

        boost::optional<QString> sitename;
        boost::optional<QString> comments;
        boost::optional<QString> investigator;
        boost::optional<double> scenerotation;
        boost::optional<QString> tape;
        boost::optional<QString> geover;
        boost::optional<QString> rdnver;
        boost::optional<double> meansceneelev;
        boost::optional<double> minsceneelev;
        boost::optional<double> maxsceneelev;
        boost::optional<int> flight;
        boost::optional<int> run;

        Scene();
    };

    typedef std::shared_ptr<Scene> ScenePtr;
}

Q_DECLARE_METATYPE(portal::ScenePtr);