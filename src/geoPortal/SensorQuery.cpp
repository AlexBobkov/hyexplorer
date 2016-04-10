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

    QStringList columns;
    columns << "ogc_fid" << "sensor" << "sceneid" << "scenetime" << "pixelsize" << "bounds" << "sunazimuth" << "sunelevation" << "hasoverview" << "hasscene" << "overviewname" << "sceneurl" << "orbitpath" << "orbitrow" << "targetpath" << "targetrow" << "processinglevel" << "satelliteinclination" << "lookangle" << "cloudmin" << "cloudmax";

    while (query.next())
    {
        ScenePtr scene = std::make_shared<Scene>();

        for (int i = 0; i < columns.size(); i++)
        {
            scene->setAttrib(columns[i], query.value(i));
        }

        QString polygonStr = scene->attrib("bounds").toString();
        scene->setGeometry(GeometryUtils::geometryFromWKT(polygonStr.toUtf8().constData()));

        scenes.push_back(scene);
    }
}

void HyperionQuery::selectScenesUnderPointer(std::set<int>& ids, const osgEarth::GeoPoint& point)
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

    QStringList columns;
    columns << "ogc_fid" << "sensor" << "sceneid" << "scenetime" << "pixelsize" << "bounds" << "sunazimuth" << "sunelevation" << "hasoverview" << "hasscene" << "overviewname" << "sceneurl" << "sitename" << "comments" << "investigator" << "scenerotation" << "tape" << "geover" << "rdnver" << "meansceneelev" << "minsceneelev" << "maxsceneelev" << "flight" << "run";

    while (query.next())
    {
        ScenePtr scene = std::make_shared<Scene>();

        for (int i = 0; i < columns.size(); i++)
        {
            scene->setAttrib(columns[i], query.value(i));
        }

        QString polygonStr = scene->attrib("bounds").toString();
        scene->setGeometry(GeometryUtils::geometryFromWKT(polygonStr.toUtf8().constData()));

        scenes.push_back(scene);        
    }
}

void AvirisQuery::selectScenesUnderPointer(std::set<int>& ids, const osgEarth::GeoPoint& point)
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
