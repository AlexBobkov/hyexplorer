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

    protected:
        osg::ref_ptr<osgViewer::View> _view;
        osg::ref_ptr<osgEarth::MapNode> _mapNode;
    };

    typedef std::shared_ptr<DataManager> DataManagerPtr;
}