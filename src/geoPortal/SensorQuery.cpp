#include "SensorQuery.hpp"

#include <osgEarthSymbology/Geometry>
#include <osgEarthFeatures/GeometryUtils>

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>

#include <vector>

using namespace osgEarth;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace portal;

SensorQuery::SensorQuery()
{
}

SensorQuery::~SensorQuery()
{
}

void SensorQuery::addCondition(const QString& str)
{
    if (_fullCondition.isEmpty())
    {
        _fullCondition = str;
        return;
    }

    _fullCondition += " AND " + str;
}

bool SensorQuery::parseCommonAttributes(const ScenePtr& scene, const QSqlQuery& query, int& c) const
{
    c = 0;
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
    scene->pixelSize = query.value(c).toDouble();

    c++;
    QString boundsStr = query.value(c).toString();

    osg::ref_ptr<Geometry> geometry = GeometryUtils::geometryFromWKT(boundsStr.toUtf8().constData());
    if (geometry->getType() == Geometry::TYPE_POLYGON && geometry->size() >= 4)
    {
        scene->swCorner = geometry->at(3);
        scene->seCorner = geometry->at(0);
        scene->neCorner = geometry->at(1);
        scene->nwCorner = geometry->at(2);

        scene->geometry = geometry;
    }
    else
    {
        qDebug() << "Geometry is not a polygon";
        return false;
    }

    //scene->feature = new Feature(geometry, _srs, Style(), scene->id);

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

    c++;
    if (query.value(c).isValid())
    {
        scene->sceneUrl = query.value(c).toString();
    }

    return true;
}

//==============================================================

void HyperionQuery::selectScenes(std::vector<ScenePtr>& scenes, const ProgressCallbackType& cb)
{  
    QString baseStr = "SELECT scenes.ogc_fid, sensor, sceneid, scenetime, pixelsize, ST_AsText(bounds), sunazimuth, sunelevation, hasoverview, hasscene, overviewname, sceneurl, orbitpath, orbitrow, targetpath, targetrow, processinglevel, satelliteinclination, lookangle, cloudmin, cloudmax "
        "FROM scenes INNER JOIN hyperion ON (scenes.ogc_fid = hyperion.ogc_fid)";

    QString queryStr;
    if (_fullCondition.isEmpty())
    {
        queryStr = baseStr + QString(" WHERE sensor='Hyperion';");
    }
    else
    {
        queryStr = baseStr + QString(" WHERE sensor='Hyperion' AND ") + _fullCondition + ";";
    }

    qDebug() << "Query " << queryStr;

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        qDebug() << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    qDebug() << "Size " << query.size();

    while (query.next())
    {
        ScenePtr scene = std::make_shared<Scene>();

        int c = 0;
        
        if (!parseCommonAttributes(scene, query, c))
        {
            continue;
        }
                
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

        scenes.push_back(scene);
    }
}

void HyperionQuery::selectScenesUnderPointer(std::set<std::size_t>& ids, const osgEarth::GeoPoint& point)
{
    QString baseStr = "SELECT scenes.ogc_fid FROM scenes INNER JOIN hyperion ON (scenes.ogc_fid = hyperion.ogc_fid)";

    QString queryStr;
    if (_fullCondition.isEmpty())
    {
        queryStr = QString(baseStr + " WHERE sensor='Hyperion' AND ST_Intersects(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'));").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }
    else
    {
        queryStr = QString(baseStr + " WHERE sensor='Hyperion' AND " + _fullCondition + " AND ST_Intersects(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'));").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        qDebug() << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    while (query.next())
    {
        ids.insert(query.value(0).toInt());
    }
}

//==============================================================

void AvirisQuery::selectScenes(std::vector<ScenePtr>& scenes, const ProgressCallbackType& cb)
{
    QString baseStr = "SELECT scenes.ogc_fid, sensor, sceneid, scenetime, pixelsize, ST_AsText(bounds), sunazimuth, sunelevation, hasoverview, hasscene, overviewname, sceneurl, sitename, comments, investigator, scenerotation, tape, geover, rdnver,  meansceneelev, minsceneelev, maxsceneelev, flight, run "
        "FROM scenes INNER JOIN aviris ON (scenes.ogc_fid = aviris.ogc_fid)";

    QString queryStr;
    if (_fullCondition.isEmpty())
    {
        queryStr = baseStr + QString(" WHERE sensor='AVIRIS';");
    }
    else
    {
        queryStr = baseStr + QString(" WHERE sensor='AVIRIS' AND ") + _fullCondition + ";";
    }

    qDebug() << "Query " << queryStr;

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        qDebug() << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    while (query.next())
    {
        ScenePtr scene = std::make_shared<Scene>();

        int c = 0;

        if (!parseCommonAttributes(scene, query, c))
        {
            continue;
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->sitename = query.value(c).toString();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->comments = query.value(c).toString();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->investigator = query.value(c).toString();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->scenerotation = query.value(c).toDouble();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->tape = query.value(c).toString();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->geover = query.value(c).toString();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->rdnver = query.value(c).toString();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->meansceneelev = query.value(c).toDouble();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->minsceneelev = query.value(c).toDouble();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->maxsceneelev = query.value(c).toDouble();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->flight = query.value(c).toInt();
        }

        c++;
        if (query.value(c).isValid())
        {
            scene->run = query.value(c).toInt();
        }

        scenes.push_back(scene);
        //_featureSource->getFeatures().push_back(scene->feature);
    }
}

void AvirisQuery::selectScenesUnderPointer(std::set<std::size_t>& ids, const osgEarth::GeoPoint& point)
{
    QString baseStr = "SELECT scenes.ogc_fid FROM scenes INNER JOIN aviris ON (scenes.ogc_fid = aviris.ogc_fid)";

    QString queryStr;
    if (_fullCondition.isEmpty())
    {
        queryStr = QString(baseStr + " WHERE sensor='AVIRIS' AND ST_Intersects(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'));").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }
    else
    {
        queryStr = QString(baseStr + " WHERE sensor='AVIRIS' AND " + _fullCondition + " AND ST_Intersects(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'));").arg(point.x(), 0, 'f', 12).arg(point.y(), 0, 'f', 12);
    }

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        qDebug() << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return;
    }

    while (query.next())
    {
        ids.insert(query.value(0).toInt());
    }
}
