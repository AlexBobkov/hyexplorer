#pragma once

#include <QDateTime>

#include <boost/optional.hpp>

#include <memory>

namespace portal
{
    struct Scene
    {
        std::string sensor;
        std::string sceneid;

        QDateTime sceneTime;

        boost::optional<int> cloundMin;
        boost::optional<int> cloundMax;

        boost::optional<int> orbitPath;
        boost::optional<int> orbitRow;

        boost::optional<int> targetPath;
        boost::optional<int> targetRow;

        boost::optional<std::string> processingLevel;

        boost::optional<double> sunAzimuth;
        boost::optional<double> sunElevation;
        boost::optional<double> inclination;
        boost::optional<double> lookAngle;
    };

    typedef std::shared_ptr<Scene> ScenePtr;
}