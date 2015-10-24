﻿#pragma once

#include <osgViewer/View>
#include <osgEarth/MapNode>

#include <memory>

namespace portal
{
    class DataManager
    {
    public:
        DataManager(osgViewer::View* view, osgEarth::MapNode* mapNode);

        osgViewer::View* view() const { return _view; }
        osgEarth::MapNode* mapNode() const { return _mapNode; }

        void updateLayer(const std::string& query);

        void setCircleNode(double lon, double lat, double radius);
        void removeCircleNode();

    protected:
        osg::ref_ptr<osgViewer::View> _view;
        osg::ref_ptr<osgEarth::MapNode> _mapNode;

        osg::observer_ptr<osgEarth::ModelLayer> _oldLayer;
        std::string _oldQuery;

        osg::ref_ptr<osg::Node> _circleNode;
    };

    typedef std::shared_ptr<DataManager> DataManagerPtr;
}