#include "MainWindow.hpp"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QApplication>
#include <QDebug>

#include <iostream>
#include <string>

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("Bobkov");
    QCoreApplication::setOrganizationDomain("alexander-bobkov.ru");
    QCoreApplication::setApplicationName("GeoPortal");

    QApplication app(argc, argv);

    MainWindow appWin;
    appWin.show();

    int result = app.exec();
    return result;
}
