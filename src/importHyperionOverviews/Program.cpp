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

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("Bobkov");
    QCoreApplication::setOrganizationDomain("alexander-bobkov.ru");
    QCoreApplication::setApplicationName("GeoPortal");

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

    QSettings settings;    
    QString storagePath = settings.value("StoragePath", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)).toString();    
    
    Downloader downloader;
    downloader.downloadFile("http://alexander-bobkov.ru/index.html");

    int result = app.exec();
    return result;
}
