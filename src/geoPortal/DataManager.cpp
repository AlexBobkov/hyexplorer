#include "DataManager.hpp"

#include <osgEarth/Viewpoint>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthAnnotation/CircleNode>
#include <osgEarthFeatures/FeatureSource>
#include <osgEarthFeatures/FeatureDisplayLayout>
#include <osgEarthSymbology/StyleSheet>
#include <osgEarthSymbology/PolygonSymbol>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>

using namespace osgEarth;
using namespace osgEarth::Drivers;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;
using namespace portal;

DataManager::DataManager(osgViewer::View* view, osgEarth::MapNode* mapNode):
_view(view),
_mapNode(mapNode)
{

}

void DataManager::setDataSet(const DataSetPtr& dataset)
{
    if (_dataset)
    {
        _mapNode->getMap()->removeModelLayer(_dataset->layer());
        _dataset = nullptr;
    }

    if (dataset)
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();

        _dataset = dataset;
        _mapNode->getMap()->addModelLayer(_dataset->layer());

        osg::Timer_t endTick = osg::Timer::instance()->tick();
        std::cout << "Loading time " << osg::Timer::instance()->delta_s(startTick, endTick) << std::endl;
    }
}

void DataManager::setCircleNode(double lon, double lat, double radius)
{
    removeCircleNode();

    Style circleStyle;
    circleStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color(Color::Red, 1.0);
    circleStyle.getOrCreate<LineSymbol>()->stroke()->width() = 4.0f;
    circleStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
    circleStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;
    circleStyle.getOrCreate<RenderSymbol>()->lighting() = false;

    _circleNode = new osgEarth::Annotation::CircleNode(_mapNode,
                                                       GeoPoint(_mapNode->getMapSRS(), lon, lat),
                                                       Linear(radius, Units::METERS),
                                                       circleStyle);
    _mapNode->addChild(_circleNode);
}

void DataManager::removeCircleNode()
{
    if (_circleNode)
    {
        _mapNode->removeChild(_circleNode);
    }
}

void DataManager::zoomToScene(const ScenePtr& scene)
{
    if (!scene)
    {
        return;
    }

    osg::Vec3d center = (scene->neCorner + scene->nwCorner + scene->seCorner + scene->swCorner) * 0.25;
        
    osgEarth::Viewpoint viewpoint;
    viewpoint.focalPoint() = GeoPoint(_mapNode->getMapSRS(), center.x(), center.y(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
    viewpoint.heading() = 0.0;
    viewpoint.pitch() = 10.0 - 90.0;
    viewpoint.range() = 150000.0;

    EarthManipulator* em = dynamic_cast<EarthManipulator*>(_view->getCameraManipulator());
    if (!em)
    {
        std::cerr << "Failed to cast to EarthManipulator\n";
        return;
    }

    em->setViewpoint(viewpoint, 3.0);
}