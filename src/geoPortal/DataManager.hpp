#pragma once

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

    protected:
        osg::ref_ptr<osgViewer::View> _view;
        osg::ref_ptr<osgEarth::MapNode> _mapNode;

        osg::observer_ptr<osgEarth::ModelLayer> _oldLayer;
        std::string _oldQuery;
    };

    typedef std::shared_ptr<DataManager> DataManagerPtr;
}