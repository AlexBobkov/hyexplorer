#pragma once

#include "DataSet.hpp"
#include "ClipInfo.hpp"

#include <osgViewer/View>
#include <osgEarth/Bounds>
#include <osgEarth/MapNode>
#include <osgEarthUtil/Sky>

#include <QStringList>
#include <QNetworkAccessManager>

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

        void addReportHandler(osgGA::GUIEventHandler* handler);
        void removeReportHandler(osgGA::GUIEventHandler* handler);

        void setDefaultActionHandler(osgGA::GUIEventHandler* handler);
        void setActionHandler(osgGA::GUIEventHandler* handler);

        void setDataSet(const DataSetPtr& dataset);

        void setCircleNode(const osgEarth::GeoPoint& center, double radius);
        void removeCircleNode();

        void zoomToScene(const ScenePtr& scene);

        const QStringList& coverageNames() const { return _coverageNames; }
        void setCoverage(const QString& coverageName);

        void showOverview(const ScenePtr& scene, const QString& filepath);

        const boost::optional<osgEarth::Bounds>& bounds() const { return _bounds; }
        void setBounds(const osgEarth::Bounds& b);

        void showScene(const ScenePtr& scene, int band, const ClipInfoPtr& clipInfo);

        QNetworkAccessManager& networkAccessManager() { return _networkManager; }

    protected:
        DataManager(const DataManager&) = delete;
        DataManager& operator=(const DataManager&) = delete;

        osg::ref_ptr<osgViewer::View> _view;
        osg::ref_ptr<osgEarth::MapNode> _mapNode;

        osg::ref_ptr<osgGA::GUIEventHandler> _defaultHandler;
        osg::ref_ptr<osgGA::GUIEventHandler> _currentHandler;

        osg::ref_ptr<osgEarth::Util::SkyNode> _sky;

        DataSetPtr _dataset;
        ScenePtr _scene;

        osg::ref_ptr<osg::Node> _circleNode;
        osg::ref_ptr<osg::Node> _overlayNode;

        boost::optional<osgEarth::Bounds> _bounds;

        QStringList _coverageNames;
        std::map<QString, osgEarth::ImageLayerOptions> _coverageMap;

        osg::ref_ptr<osgEarth::ImageLayer> _sceneLayer;

        QNetworkAccessManager _networkManager;
    };

    typedef std::shared_ptr<DataManager> DataManagerPtr;
}