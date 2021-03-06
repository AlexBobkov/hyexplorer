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

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QApplication>
#include <QDebug>
#include <QDateTime>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

const bool doInsert = true;

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <csv filename>\n";
        return 1;
    }

    std::cout << "Table structure has changed, but this importer has not.\n";
    return 1;

    QApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
#if 0
    db.setHostName("localhost");
    db.setDatabaseName("GeoPortal");
    db.setUserName("user");
    db.setPassword("user");
#else
    db.setHostName("virtualglobe.ru");
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
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }

    std::string str;
    std::getline(fin, str); //first line

    int counter = 0;

    std::ostringstream queryStream;
    queryStream << "BEGIN;";

    while (true)
    {
        std::string sceneId;
        std::getline(fin, sceneId, ',');

        if (sceneId.empty())
        {
            break; //end of file
        }

        std::string acquisitionDateStr;
        std::getline(fin, acquisitionDateStr, ',');

        QDateTime sceneTime = QDateTime::fromString(acquisitionDateStr.c_str(), "yyyy/MM/dd");

        std::string cloudStr;
        std::getline(fin, cloudStr, ',');

        boost::optional<int> cloudMin;
        boost::optional<int> cloudMax;

        if (!cloudStr.empty())
        {
            if (cloudStr[0] == '0')
            {
                cloudMin = 0;

                std::size_t index = cloudStr.find('%');
                std::string cloudMaxStr = cloudStr.substr(5, index - 5);
                cloudMax = boost::lexical_cast<int>(cloudMaxStr);
            }
            else
            {
                std::size_t indexMin = cloudStr.find('%');
                std::string cloudMinStr = cloudStr.substr(0, indexMin);
                cloudMin = boost::lexical_cast<int>(cloudMinStr);

                std::size_t indexMax = cloudStr.find('%', indexMin + 1);
                std::string cloudMaxStr = cloudStr.substr(indexMin + 5, indexMax - indexMin - 5);
                cloudMax = boost::lexical_cast<int>(cloudMaxStr);
            }
        }

        std::getline(fin, str, ',');
        int orbitPath = boost::lexical_cast<int>(str);

        std::getline(fin, str, ',');
        int orbitRow = boost::lexical_cast<int>(str);

        std::getline(fin, str, ',');
        int targetPath = boost::lexical_cast<int>(str);

        std::getline(fin, str, ',');
        int targetRow = boost::lexical_cast<int>(str);

        std::string station;
        std::getline(fin, station, ',');

        std::string processingLevel;
        std::getline(fin, processingLevel, ',');

        std::string sceneStartTimeStr;
        std::getline(fin, sceneStartTimeStr, ',');

        //����������� �� ���������� sceneStartTimeStr �����
        //������ ����������: 
        //YYYY:DDD:HH:MM:SS:SSS
        //YYYY - four - digit year
        //DDD - Julian day(001 - 366 * )(*for leap year)
        //HH - hours(00 - 23)
        //MM - minutes(00 - 59)
        //SS.SSS - seconds(00.000 - 59.999)
        std::size_t index = sceneStartTimeStr.find(':', 5);
        QTime startTime = QTime::fromString(sceneStartTimeStr.substr(index + 1).c_str(), "HH:mm:ss.zzz");
        if (!startTime.isValid())
        {
            startTime = QTime::fromString(sceneStartTimeStr.substr(index + 1).c_str(), "HH:mm:ss");
        }
        if (!startTime.isValid())
        {
            std::cerr << "Failed to parse time " << sceneStartTimeStr << std::endl;
        }
        sceneTime.setTime(startTime);
        
        std::string sceneStopTimeStr;
        std::getline(fin, sceneStopTimeStr, ',');

        std::getline(fin, str, ',');
        double sunAzimuth = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double sunElevation = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        boost::optional<double> satelliteInclination;
        if (!str.empty())
        {
            satelliteInclination = boost::lexical_cast<double>(str);
        }

        std::getline(fin, str, ',');
        double lookAngle = boost::lexical_cast<double>(str);

        std::string dateEnteredStr;
        std::getline(fin, dateEnteredStr, ',');

        std::string centerLatStr;
        std::getline(fin, centerLatStr, ',');

        std::string centerLonStr;
        std::getline(fin, centerLonStr, ',');

        std::string nwCornerLatStr;
        std::getline(fin, nwCornerLatStr, ',');

        std::string nwCornerLonStr;
        std::getline(fin, nwCornerLonStr, ',');

        std::string neCornerLatStr;
        std::getline(fin, neCornerLatStr, ',');

        std::string neCornerLonStr;
        std::getline(fin, neCornerLonStr, ',');

        std::string seCornerLatStr;
        std::getline(fin, seCornerLatStr, ',');

        std::string seCornerLonStr;
        std::getline(fin, seCornerLonStr, ',');

        std::string swCornerLatStr;
        std::getline(fin, swCornerLatStr, ',');

        std::string swCornerLonStr;
        std::getline(fin, swCornerLonStr, ',');

        std::getline(fin, str, ',');
        double centerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double centerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double nwCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double nwCornerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double neCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double neCornerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double seCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double seCornerLon = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double swCornerLat = boost::lexical_cast<double>(str);

        std::getline(fin, str, ',');
        double swCornerLon = boost::lexical_cast<double>(str);

        std::ostringstream polygonStream;
        polygonStream << std::setprecision(15);
        polygonStream << "ST_GeographyFromText('SRID=4326;POLYGON((";
        polygonStream << swCornerLon << " " << swCornerLat << ",";
        polygonStream << seCornerLon << " " << seCornerLat << ",";
        polygonStream << neCornerLon << " " << neCornerLat << ",";
        polygonStream << nwCornerLon << " " << nwCornerLat << ",";
        polygonStream << swCornerLon << " " << swCornerLat << "))')";

        std::string displayId;
        std::getline(fin, displayId, ',');

        std::string orderingId;
        std::getline(fin, orderingId, ',');

        std::string downloadLink;
        std::getline(fin, downloadLink);

        if (fin.fail())
        {
            std::cout << "Failed to read file " << argv[1] << std::endl;
            return 1;
        }

        queryStream << "INSERT INTO public.scenes (sensor, sceneid, orbitpath, orbitrow, targetpath, targetrow, processinglevel, sunazimuth, sunelevation, lookangle, scenetime, bounds, hasoverview, hasscene";
        if (cloudMin && cloudMax)
        {
            queryStream << ", cloudMin, cloudMax";
        }
        if (satelliteInclination)
        {
            queryStream << ", satelliteinclination";
        }
        queryStream << ") ";

        queryStream << "VALUES('Hyperion','" << sceneId << "'," << orbitPath << "," << orbitRow << "," << targetPath << "," << targetRow << ",'" << processingLevel << "'," << sunAzimuth << "," << sunElevation << "," << lookAngle << ",'" << sceneTime.toString(Qt::ISODate).toUtf8().constData() << "'," << polygonStream.str() << ",FALSE,FALSE";
        if (cloudMin && cloudMax)
        {
            queryStream << "," << *cloudMin << "," << *cloudMax;
        }
        if (satelliteInclination)
        {
            queryStream << "," << *satelliteInclination;
        }
        queryStream << "); ";

        counter++;

        if (counter % 10000 == 0)
        {
            std::cout << "Insert counter " << counter << std::endl;

            queryStream << "COMMIT;ANALYZE public.scenes;";

            if (doInsert)
            {
                QSqlQuery query;
                if (!query.exec(queryStream.str().c_str()))
                {
                    qDebug() << "Failed to execute insert query: " << query.lastError().text();
                    return 1;
                }
            }

            queryStream = std::ostringstream();
            queryStream << "BEGIN;";
        }
    }

    std::cout << "Insert counter " << counter << std::endl;

    queryStream << "COMMIT;ANALYZE public.scenes;";

    if (doInsert)
    {
        QSqlQuery query;
        if (!query.exec(queryStream.str().c_str()))
        {
            qDebug() << "Failed to execute insert query: " << query.lastError().text();
            return 1;
        }
    }

    fin.close();
     
    std::cout << "Records " << counter << std::endl;

    return 0;
}
