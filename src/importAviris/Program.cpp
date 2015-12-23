#include <AvirisCsvRow.hpp>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QApplication>
#include <QDebug>
#include <QDateTime>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <set>

const bool doInsert = true;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        qDebug() << "Usage: " << argv[0] << " <csv filename>";
        return 1;
    }

    QApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
#if 0
    db.setHostName("localhost");
    db.setDatabaseName("GeoPortal");
    db.setUserName("user");
    db.setPassword("user");
#else
    db.setHostName("178.62.140.44");
    db.setDatabaseName("GeoPortal");
    db.setUserName("portal");
    db.setPassword("PortalPass");
#endif

    if (!db.open())
    {
        qDebug() << "Failed to open database:" << db.lastError().text();
        return 1;
    }

    std::ifstream fin(argv[1]);
    if (!fin)
    {
        qDebug() << "Failed to open file " << argv[1];
        return 1;
    }

    std::string str;
    std::getline(fin, str); //first line

    int totalCounter = 0;

    std::set<QString> sceneIdSet;
    std::vector<QString> scenesOrder;
    std::map<QString, double> sceneIdToFileSize;
    std::map<QString, QString> sceneIdToQuery;

    while (true)
    {
        std::string str;
        std::getline(fin, str);
        if (str.empty())
        {
            break; //end of file
        }

        CsvRow row(str);

        QString wrongSceneId = row.as<QString>(0);
        QString sitename = row.as<QString>(1);
        QString investigator = row.as<QString>(3);
        QString comments = row.as<QString>(4);

        QString flightScene = row.as<QString>(5);

        QString rdnver = row.as<QString>(7);
        QString geover = row.as<QString>(9);

        QString tape = row.as<QString>(11);
        int flight = row.as<int>(13);
        int run = row.as<int>(14);

        int year = row.as<int>(15);
        int month = row.as<int>(16);
        int day = row.as<int>(17);
        int hour = row.as<int>(18);
        int minute = row.as<int>(19);

        QDateTime sceneTime = QDateTime(QDate(year, month, day), QTime(hour, minute), Qt::UTC);

        double pixelSize = row.as<double>(20);
        double rotation = row.as<double>(21);

        double sunElevation = row.as<double>(24);
        double sunAzimuth = row.as<double>(25);

        double meanSceneElev = row.as<double>(26);
        double minSceneElev = row.as<double>(27);
        double maxSceneElev = row.as<double>(28);

        double fileSize = row.as<double>(32);

        QString sceneUrl = row.as<QString>(38);

        double lon1 = row.as<double>(40);
        double lon2 = row.as<double>(41);
        double lon3 = row.as<double>(42);
        double lon4 = row.as<double>(43);

        double lat1 = row.as<double>(44);
        double lat2 = row.as<double>(45);
        double lat3 = row.as<double>(46);
        double lat4 = row.as<double>(47);

        QString polygonStr = QString("ST_GeographyFromText('SRID=4326;POLYGON((%0 %1, %2 %3, %4 %5, %6 %7, %0 %1))')")
            .arg(lon1, 0, 'f', 12).arg(lat1, 0, 'f', 12)
            .arg(lon2, 0, 'f', 12).arg(lat2, 0, 'f', 12)
            .arg(lon3, 0, 'f', 12).arg(lat3, 0, 'f', 12)
            .arg(lon4, 0, 'f', 12).arg(lat4, 0, 'f', 12);

        if (fin.fail())
        {
            qDebug() << "Failed to read file " << argv[1];
            return 1;
        }

        QString queryStr = QString("WITH ins1 AS ("
                                   "INSERT INTO public.scenes (sensor, sceneid, scenetime, pixelsize, bounds, sunazimuth, sunelevation, hasoverview, hasscene, sceneurl) "
                                   "VALUES('AVIRIS', '%0', '%1', %2, %3, %4, %5, FALSE, FALSE, '%6') "
                                   "RETURNING ogc_fid"
                                   ") "
                                   "INSERT INTO public.aviris (ogc_fid, sitename, comments, investigator, scenerotation, tape, geover, rdnver, meansceneelev, minsceneelev, maxsceneelev, flight, run) "
                                   "SELECT ogc_fid, '%7', '%8', '%9', %10, '%11', '%12', '%13', %14, %15, %16, %17, %18 "
                                   "FROM ins1;")
                                   .arg(flightScene)
                                   .arg(sceneTime.toString(Qt::ISODate))
                                   .arg(pixelSize, 0, 'f', 10)
                                   .arg(polygonStr)
                                   .arg(sunAzimuth, 0, 'f', 10)
                                   .arg(sunElevation, 0, 'f', 10)
                                   .arg(sceneUrl)
                                   .arg(sitename)
                                   .arg(comments)
                                   .arg(investigator)
                                   .arg(rotation, 0, 'f', 10)
                                   .arg(tape)
                                   .arg(geover)
                                   .arg(rdnver)
                                   .arg(meanSceneElev, 0, 'f', 10)
                                   .arg(minSceneElev, 0, 'f', 10)
                                   .arg(maxSceneElev, 0, 'f', 10)
                                   .arg(flight)
                                   .arg(run);

        totalCounter++;

        if (sceneIdSet.count(flightScene) == 0)
        {
            sceneIdSet.insert(flightScene);
            scenesOrder.push_back(flightScene);

            sceneIdToFileSize[flightScene] = fileSize;
            sceneIdToQuery[flightScene] = queryStr;            
        }
        else
        {
            if (sceneIdToFileSize[flightScene] > 0)
            {
                qDebug() << "Duplicate " << flightScene << " size " << fileSize << " prev size " << sceneIdToFileSize[flightScene];
                continue;
            }
            else
            {
                sceneIdToFileSize[flightScene] = fileSize;
                sceneIdToQuery[flightScene] = queryStr;

                qDebug() << "Replace " << flightScene;
                continue;
            }
        }
    }

    assert(sceneIdToQuery.size() == scenesOrder.size());
    assert(sceneIdToQuery.size() == sceneIdSet.size());

    qDebug() << "Total rows " << totalCounter << " distinct " << sceneIdToQuery.size();

    QString queryStream;
    queryStream = "BEGIN;";

    int counter = 0;
    for (; counter < scenesOrder.size(); counter++)
    {
        queryStream += sceneIdToQuery[scenesOrder[counter]];

        if (counter > 0 && (counter % 1000 == 0 || counter == scenesOrder.size() - 1))
        {
            qDebug() << "Insert counter " << counter;

            queryStream += "COMMIT;ANALYZE public.scenes;ANALYZE public.aviris;";

            if (doInsert)
            {
                qDebug() << queryStream;

                QSqlQuery query;
                if (!query.exec(queryStream))
                {
                    qDebug() << "Failed to execute insert query: " << query.lastError().text();
                    return 1;
                }
            }

            queryStream = "BEGIN;";
        }
    }    

    fin.close();

    return 0;
}
