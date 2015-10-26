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

#include <iostream>
#include <vector>

using namespace osgEarth;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace osgEarth::Drivers;
using namespace portal;

void DataSet::addCondition(const QString& str)
{
    if (_fullCondition.isEmpty())
    {
        _fullCondition = str;
        return;
    }

    _fullCondition += " and " + str;
}

void DataSet::selectScenes()
{
    QString queryStr;

    if (_fullCondition.isEmpty())
    {
        queryStr = "select ogc_fid, sensor, sceneid, orbitpath, orbitrow, targetpath, targetrow, processinglevel, sunazimuth, sunelevation, satelliteinclination, lookangle, scenetime, cloudmin, cloudmax, ST_AsText(bounds) from scenes;";
    }
    else
    {
        queryStr = "select ogc_fid, sensor, sceneid, orbitpath, orbitrow, targetpath, targetrow, processinglevel, sunazimuth, sunelevation, satelliteinclination, lookangle, scenetime, cloudmin, cloudmax, ST_AsText(bounds) from scenes where " + _fullCondition + ";";
    }

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        std::cerr << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    osgEarth::SpatialReference* srs = osgEarth::SpatialReference::get("wgs84");

    FeatureListSource* featureListSource = new FeatureListSource;
    
    while (query.next())
    {
        ScenePtr scene = std::make_shared<Scene>();

        scene->id = query.value(0).toInt();
        scene->sensor = query.value(1).toString();
        scene->sceneid = query.value(2).toString();

        if (query.value(3).isValid())
        {
            scene->orbitPath = query.value(3).toInt();
        }

        if (query.value(4).isValid())
        {
            scene->orbitRow = query.value(4).toInt();
        }

        if (query.value(5).isValid())
        {
            scene->targetPath = query.value(5).toInt();
        }

        if (query.value(6).isValid())
        {
            scene->targetRow = query.value(6).toInt();
        }

        if (query.value(7).isValid())
        {
            scene->processingLevel = query.value(7).toString();
        }

        if (query.value(8).isValid())
        {
            scene->sunAzimuth = query.value(8).toDouble();
        }

        if (query.value(9).isValid())
        {
            scene->sunElevation = query.value(9).toDouble();
        }

        if (query.value(10).isValid())
        {
            scene->inclination = query.value(10).toDouble();
        }

        if (query.value(11).isValid())
        {
            scene->lookAngle = query.value(11).toDouble();
        }

        if (query.value(12).isValid())
        {
            scene->sceneTime = query.value(12).toDateTime();
        }

        if (query.value(13).isValid())
        {
            scene->cloundMin = query.value(13).toInt();
        }

        if (query.value(14).isValid())
        {
            scene->cloundMax = query.value(14).toInt();
        }

        QString boundsStr = query.value(15).toString();
                
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
            std::cerr << "Geometry is not a polygon\n";
            continue;
        }

        scene->feature = new Feature(geometry, srs, Style(), scene->id);

        _scenes.push_back(scene);
        featureListSource->getFeatures().push_back(scene->feature);
    }

    _featureSource = featureListSource;

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
    
    FeatureGeomModelOptions fgmOpt;
    fgmOpt.featureSource() = _featureSource;
    fgmOpt.styles() = styleSheet;
    
    _layer = new ModelLayer("scenes", fgmOpt);

    std::cout << "Scenes found " << _scenes.size() << std::endl;
}

void DataSet::selectScenesUnderPointer(const osgEarth::GeoPoint& point)
{
    QString queryStr;

    if (_fullCondition.isEmpty())
    {
        queryStr = QString("select ogc_fid from scenes where bounds && ST_GeographyFromText('SRID=4326;POINT(%0 %1)');").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }
    else
    {
        queryStr = QString("select ogc_fid from scenes where " + _fullCondition + " and bounds && ST_GeographyFromText('SRID=4326;POINT(%0 %1)');").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        std::cerr << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    std::set<std::size_t> ids;

    while (query.next())
    {
        ids.insert(query.value(0).toInt());
    }

    _sceneIdsUnderPointer = ids;

    std::cout << "Scenes under pointer found " << ids.size() << std::endl;
}

bool DataSet::isSceneUnderPointer(const ScenePtr& scene) const
{
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