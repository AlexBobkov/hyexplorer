/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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

    std::set<int> ids;

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

    if (_sceneIdsUnderPointer.find(scene->id()) != _sceneIdsUnderPointer.end())
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
        featureSource->getFeatures().push_back(new Feature(s->geometry(), _srs, Style(), s->id()));
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