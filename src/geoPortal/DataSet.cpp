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

}

void DataSet::addCondition(const QString& str)
{
    if (_initialized)
    {
        throw std::runtime_error("DataSet is already initialized");
    }

    if (_fullCondition.isEmpty())
    {
        _fullCondition = str;
        return;
    }

    _fullCondition += " and " + str;
}

void DataSet::selectScenes(const ProgressCallbackType& cb)
{
    if (_initialized)
    {
        throw std::runtime_error("DataSet is already initialized");
    }

    QString baseStr = "select scenes.ogc_fid, sensor, sceneid, scenetime, ST_AsText(bounds), sunazimuth, sunelevation, hasoverview, hasscene, overviewname, orbitpath, orbitrow, targetpath, targetrow, processinglevel, satelliteinclination, lookangle, cloudmin, cloudmax from scenes inner join hyperion on (scenes.ogc_fid = hyperion.ogc_fid)";

    QString queryStr;
    if (_fullCondition.isEmpty())
    {
        queryStr = baseStr + ";";
    }
    else
    {
        queryStr = baseStr + " where " + _fullCondition + ";";
    }

    qDebug() << "Query " << queryStr;

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        qDebug() << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    osgEarth::SpatialReference* srs = osgEarth::SpatialReference::get("wgs84");

    FeatureListSource* featureListSource = new FeatureListSource;
    
    while (query.next())
    {
        ScenePtr scene = std::make_shared<Scene>();

        int c = 0;
        scene->id = query.value(c).toInt();

        c++;
        scene->sensor = query.value(c).toString();

        c++;
        scene->sceneId = query.value(c).toString();

        c++;
        if (query.value(c).isValid())
        {
            scene->sceneTime = query.value(c).toDateTime();
        }

        c++;
        QString boundsStr = query.value(c).toString();

        osg::ref_ptr<Geometry> geometry = GeometryUtils::geometryFromWKT(boundsStr.toUtf8().constData());
        if (geometry->getType() == Geometry::TYPE_POLYGON && geometry->size() >= 4)
        {
            scene->swCorner = geometry->at(3);
            scene->seCorner = geometry->at(0);
            scene->neCorner = geometry->at(1);
            scene->nwCorner = geometry->at(2);
        }
        else
        {
            qDebug() << "Geometry is not a polygon";
            continue;
        }

        scene->feature = new Feature(geometry, srs, Style(), scene->id);

        c++;
        if (query.value(c).isValid())
        {
            scene->sunAzimuth = query.value(c).toDouble();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->sunElevation = query.value(c).toDouble();
        }

        c++;
        scene->hasOverview = query.value(c).toBool();

        c++;
        scene->hasScene = query.value(c).toBool();

        c++;
        if (query.value(c).isValid())
        {
            scene->overviewName = query.value(c).toString();
        }

        //-- Hyperion

        c++;
        if (query.value(c).isValid())
        {
            scene->orbitPath = query.value(c).toInt();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->orbitRow = query.value(c).toInt();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->targetPath = query.value(c).toInt();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->targetRow = query.value(c).toInt();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->processingLevel = query.value(c).toString();
        }
        
        c++;
        if (query.value(c).isValid())
        {
            scene->inclination = query.value(c).toDouble();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->lookAngle = query.value(c).toDouble();
        }        

        c++;
        if (query.value(c).isValid())
        {
            scene->cloundMin = query.value(c).toInt();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->cloundMax = query.value(c).toInt();
        }

        _scenes.push_back(scene);
        featureListSource->getFeatures().push_back(scene->feature);
    }

    _featureSource = featureListSource;

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
    fgmOpt.featureSource() = _featureSource;
    fgmOpt.styles() = styleSheet;
    //fgmOpt.layout() = layout;
    
    _layer = new ModelLayer("scenes", fgmOpt);

    _initialized = true;

    qDebug() << "Scenes found " << _scenes.size();
}

void DataSet::selectScenesUnderPointer(const osgEarth::GeoPoint& point)
{
    if (!_initialized)
    {
        throw std::runtime_error("DataSet is not initialized");
    }

    QString baseStr = "select scenes.ogc_fid from scenes inner join hyperion on (scenes.ogc_fid = hyperion.ogc_fid)";

    QString queryStr;
    if (_fullCondition.isEmpty())
    {
        queryStr = QString(baseStr + " where ST_Intersects(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'));").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }
    else
    {
        queryStr = QString(baseStr + " where " + _fullCondition + " and ST_Intersects(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'));").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        qDebug() << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    std::set<std::size_t> ids;

    while (query.next())
    {
        ids.insert(query.value(0).toInt());
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