#pragma once

#include "DataSet.hpp"

#include <osgViewer/View>
#include <osgEarth/MapNode>
#include <osgEarthUtil/Sky>

#include <memory>

namespace portal
{
    class DataManager
    {
    public:
        DataManager(osgViewer::View* view, osgEarth::MapNode* mapNode);

        osgViewer::View* view() const { return _view; }
        osgEarth::MapNode* mapNode() const { return _mapNode; }

        bool atmosphereVisibility() const;
        void setAtmosphereVisibility(bool b);
        
        void setDataSet(const DataSetPtr& dataset);

        void setCircleNode(const osgEarth::GeoPoint& center, double radius);
        void removeCircleNode();

        void zoomToScene(const ScenePtr& scene);

    protected:
        DataManager(const DataManager&) = delete;
        DataManager& operator=(const DataManager&) = delete;

        osg::ref_ptr<osgViewer::View> _view;
        osg::ref_ptr<osgEarth::MapNode> _mapNode;

        osg::ref_ptr<osgEarth::Util::SkyNode> _sky;

        osg::observer_ptr<osgEarth::ModelLayer> _oldLayer;
        std::string _oldQuery;

        DataSetPtr _dataset;

        osg::ref_ptr<osg::Node> _circleNode;
    };

    typedef std::shared_ptr<DataManager> DataManagerPtr;
}