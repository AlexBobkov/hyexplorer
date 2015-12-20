#include "DataSet.hpp"

#include <osgEarthSymbology/Geometry>
#include <osgEarthFeatures/Feature>
#include <osgEarthFeatures/FeatureListSource>
#include <osgEarthFeatures/GeometryUtils>
#include <osgEarthFeatures/FeatureDisplayLayout>
#include <osgEarthSymbology/StyleSheet>
#include <osgEarthSymbology/PolygonSymbol>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

#include <vector>

using namespace osgEarth;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace osgEarth::Drivers;
using namespace portal;

DataSet::DataSet():
_initialized(false)
{      
    _srs = osgEarth::SpatialReference::get("wgs84");
}

void DataSet::addSensor(const SensorQueryPtr& sensorQuery)
{
    _sensors.push_back(sensorQuery);
}

void DataSet::selectScenes(const ProgressCallbackType& cb)
{
    if (_initialized)
    {
        throw std::runtime_error("DataSet is already initialized");
    }
        
    for (const auto& s : _sensors)
    {
        s->selectScenes(_scenes, cb);
    }
        
    _initialized = true;

    qDebug() << "Scenes found " << _scenes.size();
}

void DataSet::selectScenesUnderPointer(const osgEarth::GeoPoint& point)
{
    if (!_initialized)
    {
        throw std::runtime_error("DataSet is not initialized");
    }

    std::set<std::size_t> ids;

    for (const auto& s : _sensors)
    {
        s->selectScenesUnderPointer(ids, point);
    }

    _sceneIdsUnderPointer = ids;

    qDebug() << "Scenes under pointer found " << ids.size();
}

bool DataSet::isSceneUnderPointer(const ScenePtr& scene) const
{
    if (!_initialized)
    {
        throw std::runtime_error("DataSet is not initialized");
    }

    if (!scene)
    {
        return false;
    }

    if (_sceneIdsUnderPointer.find(scene->id) != _sceneIdsUnderPointer.end())
    {
        return true;
    }

    return false;
}

osg::ref_ptr<osgEarth::ModelLayer> DataSet::getOrCreateLayer()
{
    if (!_initialized)
    {
        throw std::runtime_error("DataSet is not initialized");
    }

    if (_layer)
    {
        return _layer;
    }

    osg::ref_ptr<osgEarth::Features::FeatureListSource> featureSource = new FeatureListSource;

    for (const auto& s : _scenes)
    {
        featureSource->getFeatures().push_back(new Feature(s->geometry, _srs, Style(), s->id));
    }

    Style style;

    PolygonSymbol* poly = style.getOrCreate<PolygonSymbol>();
    poly->fill()->color() = Color(Color::Yellow, 0.8f);

    LineSymbol* line = style.getOrCreate<LineSymbol>();
    line->stroke()->color() = Color::Black;
    line->stroke()->width() = 150.0;
    line->stroke()->widthUnits() = osgEarth::Units::METERS;

    AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;
    alt->technique() = alt->TECHNIQUE_DRAPE;

    RenderSymbol* render = style.getOrCreate<RenderSymbol>();
    render->depthTest() = false;

    StyleSheet* styleSheet = new StyleSheet();
    styleSheet->addStyle(style);

    //osgEarth::Features::FeatureDisplayLayout layout;
    //layout.tileSizeFactor() = 5.0f;
    //layout.addLevel(osgEarth::Features::FeatureLevel(0, 50000000));

    FeatureGeomModelOptions fgmOpt;
    fgmOpt.featureSource() = featureSource;
    fgmOpt.styles() = styleSheet;
    //fgmOpt.layout() = layout;

    _layer = new ModelLayer("scenes", fgmOpt);

    return _layer;
}