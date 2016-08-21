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

#include "Downloader.hpp"

#include <AvirisCsvRow.hpp>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QApplication>
#include <QDebug>
#include <QDateTime>
#include <QSettings>
#include <QStandardPaths>

#include <boost/lexical_cast.hpp>
#include <boost/optional.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <queue>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        std::cout << "Usage: " << argv[0] << " <csv filename>\n";
        return 1;
    }

    QCoreApplication::setOrganizationName("Bobkov");
    QCoreApplication::setOrganizationDomain("alexander-bobkov.ru");
    QCoreApplication::setApplicationName("GeoPortal");

    QApplication app(argc, argv);

    QSettings settings;
    QString storagePath = settings.value("StoragePath", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString();

    Downloader downloader;

    //------------------------------------------

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

    //------------------------------------------

    std::map<QString, QString> overviewMap;

    std::ifstream fin(argv[1]);
    if (!fin)
    {
        std::cerr << "Failed to open file " << argv[1] << std::endl;
        return 1;
    }

    std::string str;
    std::getline(fin, str); //first line

    while (true)
    {
        std::string str;
        std::getline(fin, str);
        if (str.empty())
        {
            break; //end of file
        }

        CsvRow row(str);

        QString flightScene = row.as<QString>(5);
        QString sceneUrl = row.as<QString>(37);

        overviewMap[flightScene] = sceneUrl;
    }

    //------------------------------------------
        
    QString queryStr = "select sceneid from scenes where sensor='AVIRIS' and not hasoverview limit 1000;";

    QSqlQuery query;
    if (!query.exec(queryStr))
    {
        std::cerr << "Failed to exececute query " << qPrintable(queryStr) << " error " << qPrintable(query.lastError().text());
        return 1;
    }

    std::queue<QString> scenes;

    while (query.next())
    {
        QString sceneid = query.value(0).toString();
        scenes.push(sceneid);
    }

    if (scenes.empty())
    {
        std::cerr << "No scenes found\n";
        return 1;
    }

    std::cout << "Scenes found: " << scenes.size() << std::endl;

    downloader.setQueue(scenes, overviewMap);

    int result = app.exec();
    return result;
}
