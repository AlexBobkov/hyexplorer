#include "MainWindow.hpp"
#include "Scene.hpp"

#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osgEarth/MapNode>
#include <osgEarth/ObjectIndex>
#include <osgEarth/Registry>
#include <osgEarth/Capabilities>
#include <osgEarthFeatures/FeatureIndex>
#include <osgEarthQt/ViewerWidget>
#include <osgEarthUtil/RTTPicker>
#include <osgEarthUtil/EarthManipulator>

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QApplication>
#include <QDebug>
#include <QMessageBox>

#include <string>

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Util;
using namespace portal;

int main(int argc, char** argv)
{
    srand(time(NULL));

    QCoreApplication::setOrganizationName("MIIGAiK");
    QCoreApplication::setOrganizationDomain("miigaik.ru");
    QCoreApplication::setApplicationName("HyExplorer");

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

    //---------------------------------------------------

    int cores = osgEarth::Registry::capabilities().getNumProcessors();
    osg::DisplaySettings::instance()->setNumOfDatabaseThreadsHint(osg::clampAbove(cores, 2));
    osg::DisplaySettings::instance()->setNumOfHttpDatabaseThreadsHint(osg::clampAbove(cores / 2, 1));
    
    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("data/globe.earth");
    osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(node);

    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild(node);

    osgEarth::QtGui::ViewerWidget* viewerWidget = new osgEarth::QtGui::ViewerWidget(root);

    osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(viewerWidget->getViewer());
    if (!viewer)
    {
        qDebug() << "Wrong viewer";
        return 1;
    }

    viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

    EarthManipulator* earthManipulator = dynamic_cast<EarthManipulator*>(viewer->getCameraManipulator());
    earthManipulator->getSettings()->setScrollSensitivity(-1.0);
   
    DataManagerPtr dataManager = std::make_shared<DataManager>(viewer, mapNode);

    //---------------------------------------------------

    MainWindow appWin(dataManager);
    appWin.setCentralWidget(viewerWidget);
        
    QSettings settings;    
    if (!settings.contains("StoragePath"))
    {
        settings.setValue("StoragePath", QStandardPaths::writableLocation(QStandardPaths::AppDataLocation));
    }
    
    appWin.restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    //appWin.restoreState(settings.value("MainWindow/windowState").toByteArray());

    appWin.show();

    return app.exec();
}
