#include "MainWindow.hpp"

#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osgEarth/MapNode>
#include <osgEarthQt/ViewerWidget>

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
    
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("data/globe.earth");
    osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(node);

    osgEarth::QtGui::ViewerWidget* viewerWidget = new osgEarth::QtGui::ViewerWidget(node);
    viewerWidget->getViewer()->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);
    appWin.setCentralWidget(viewerWidget);

    appWin.setMapNode(mapNode);

    appWin.show();

    int result = app.exec();
    return result;
}
