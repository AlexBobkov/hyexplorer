#include "DataManager.hpp"

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
using namespace portal;

DataManager::DataManager(osgViewer::View* view, osgEarth::MapNode* mapNode):
_view(view),
_mapNode(mapNode)
{

}

void DataManager::updateLayer(const std::string& query)
{
    if (_oldLayer.valid() && query == _oldQuery)
    {
        return;
    }

    if (_oldLayer.valid())
    {
        _mapNode->getMap()->removeModelLayer(_oldLayer.get());
        _oldLayer = nullptr;
    }

    std::cout << "Query " << query << std::endl;

    OGRFeatureOptions featureOpt;
    featureOpt.ogrDriver() = "PostgreSQL";
#if 0
    featureOpt.connection() = "PG:dbname='GeoPortal' host='178.62.140.44' port='5432' user='portal' password='PortalPass'";
#else
    featureOpt.connection() = "PG:dbname='GeoPortal' host='localhost' port='5432' user='user' password='user'";
#endif
    featureOpt.layer() = "scenes";
    featureOpt.buildSpatialIndex() = true;

    Style style;

    PolygonSymbol* poly = style.getOrCreate<PolygonSymbol>();
    poly->fill()->color() = Color::Yellow;

    LineSymbol* line = style.getOrCreate<LineSymbol>();
    line->stroke()->color() = Color::Black;
    line->stroke()->width() = 150.0;
    line->stroke()->widthUnits() = osgEarth::Units::METERS;

    AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;
    alt->technique() = alt->TECHNIQUE_DRAPE;

    StyleSheet* styleSheet = new StyleSheet();
    styleSheet->addStyle(style);

    if (!query.empty())
    {
        Query q;
        q.expression() = query;

        osgEarth::Symbology::StyleSelector selector;
        selector.query() = q;
        styleSheet->selectors().push_back(selector);
    }

    //FeatureDisplayLayout layout;
    //layout.tileSizeFactor() = 52.0;
    //layout.addLevel(FeatureLevel(0.0f, 20000.0f, "buildings"));

    FeatureGeomModelOptions fgmOpt;
    fgmOpt.featureOptions() = featureOpt;
    fgmOpt.styles() = styleSheet;
    //fgmOpt.layout() = layout;

    _oldLayer = new ModelLayer("scenes", fgmOpt);
    _oldQuery = query;

    osg::Timer_t startTick = osg::Timer::instance()->tick();

    _mapNode->getMap()->addModelLayer(_oldLayer.get());

    osg::Timer_t endTick = osg::Timer::instance()->tick();
    std::cout << "Loading time " << osg::Timer::instance()->delta_s(startTick, endTick) << std::endl;

    //int fc = dynamic_cast<osgEarth::Features::FeatureModelSource*>(_oldLayer->getModelSource())->getFeatureSource()->getFeatureCount();
    //std::cout << "Count = " << fc << std::endl;
}