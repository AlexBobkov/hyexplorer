#include "Downloader.hpp"

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

    //------------------------------------------
    
    QString queryStr = "select sceneid from scenes where sensor='Hyperion' and not hasoverview limit 5;";

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

    downloader.process(scenes);

    int result = app.exec();
    return result;
}
