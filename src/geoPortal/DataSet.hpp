#pragma once

#include <osg/Vec3d>

#include <osgEarth/MapNode>
#include <osgEarthFeatures/FeatureSource>

#include <QDateTime>
#include <QString>

#include <boost/optional.hpp>

#include <memory>

namespace portal
{
    class DataSet
    {
    public:
        QString fullCondition() const { return _fullCondition; }
        void addCondition(const QString& str);
        
        void execute();

        osgEarth::ModelLayer* layer() const { return _layer; }

    protected:
        QString _fullCondition;

        osg::ref_ptr<osgEarth::Features::FeatureSource> _featureSource;
        osg::ref_ptr<osgEarth::ModelLayer> _layer;
    };

    typedef std::shared_ptr<DataSet> DataSetPtr;
}