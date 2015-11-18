#pragma once

#include "DataSet.hpp"

#include <osgViewer/View>
#include <osgEarth/Bounds>
#include <osgEarth/MapNode>
#include <osgEarthUtil/Sky>

#include <QStringList>

#include <boost/optional.hpp>

#include <memory>
#include <vector>
#include <map>

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

        const QStringList& coverageNames() const { return _coverageNames; }
        void setCoverage(const QString& coverageName);

        void showOverview(const ScenePtr& scene, const QString& filepath);

        const boost::optional<osgEarth::Bounds>& rectangle() const { return _rectangle; }
        void setRectangle(const osgEarth::Bounds& b) { _rectangle = b; }

        void showScene(const ScenePtr& scene);
        void setActiveBand(int band);
        void setClipMode(bool b);

    protected:
        DataManager(const DataManager&) = delete;
        DataManager& operator=(const DataManager&) = delete;

        osg::ref_ptr<osgViewer::View> _view;
        osg::ref_ptr<osgEarth::MapNode> _mapNode;

        osg::ref_ptr<osgEarth::Util::SkyNode> _sky;
        
        DataSetPtr _dataset;
        ScenePtr _scene;

        osg::ref_ptr<osg::Node> _circleNode;
        osg::ref_ptr<osg::Node> _overlayNode;

        boost::optional<osgEarth::Bounds> _rectangle;

        QStringList _coverageNames;
        std::map<QString, osgEarth::ImageLayerOptions> _coverageMap;

        osg::ref_ptr<osgEarth::ImageLayer> _sceneLayer;

        int _activeBand;
        bool _clipMode;
    };

    typedef std::shared_ptr<DataManager> DataManagerPtr;
}