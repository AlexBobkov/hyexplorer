#include "MainWindow.hpp"
#include "Scene.hpp"

#include <osg/ArgumentParser>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>

#include <osgEarth/MapNode>
#include <osgEarth/ObjectIndex>
#include <osgEarth/Registry>
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

#include <iostream>
#include <string>

using namespace osgEarth;
using namespace osgEarth::Symbology;
using namespace osgEarth::Features;
using namespace osgEarth::Util;
using namespace portal;

namespace
{
    struct MyPickCallback : public RTTPicker::Callback
    {
        osg::ref_ptr<osg::Uniform> highlightUniform;
        MainWindow* mainWindow;

        MyPickCallback(osg::Uniform* uniform, MainWindow* mainWindow) :
            highlightUniform(uniform),
            mainWindow(mainWindow)
        {}

        void onHit(ObjectID id)
        {
            FeatureIndex* index = Registry::objectIndex()->get<FeatureIndex>(id);
            Feature* feature = index ? index->getFeature(id) : 0L;

            highlightUniform->set(id);
        }

        void onMiss()
        {
            highlightUniform->set(0u);
        }

        bool accept(const osgGA::GUIEventAdapter& ea, const osgGA::GUIActionAdapter& aa)
        {
            return ea.getEventType() == ea.RELEASE;
        }
    };

    const char* highlightVert =
        "#version " GLSL_VERSION_STR "\n"
        "uniform uint objectid_to_highlight; \n"
        "uint oe_index_objectid;      // Stage global containing object id \n"
        "flat out int selected; \n"
        "void checkForHighlight(inout vec4 vertex) \n"
        "{ \n"
        "    selected = (objectid_to_highlight > 1u && objectid_to_highlight == oe_index_objectid) ? 1 : 0; \n"
        "} \n";

    const char* highlightFrag =
        "#version " GLSL_VERSION_STR "\n"
        "flat in int selected; \n"
        "void highlightFragment(inout vec4 color) \n"
        "{ \n"
        "    if ( selected == 1 ) \n"
        "        color.rgb = mix(color.rgb, clamp(vec3(0.5,0.5,2.0)*(1.0-color.rgb), 0.0, 1.0), 0.5); \n"
        "} \n";

    osg::Uniform* installHighlighter(osg::StateSet* stateSet, int attrLocation)
    {
        // This shader program will highlight the selected object.
        VirtualProgram* vp = VirtualProgram::getOrCreate(stateSet);
        vp->setFunction("checkForHighlight", highlightVert, ShaderComp::LOCATION_VERTEX_CLIP);
        vp->setFunction("highlightFragment", highlightFrag, ShaderComp::LOCATION_FRAGMENT_COLORING);

        // Since we're accessing object IDs, we need to load the indexing shader as well:
        Registry::objectIndex()->loadShaders(vp);

        // A uniform that will tell the shader which object to highlight:
        osg::Uniform* uniform = new osg::Uniform("objectid_to_highlight", 0u);
        stateSet->addUniform(uniform);

        return uniform;
    }
}

int main(int argc, char** argv)
{
    QCoreApplication::setOrganizationName("Bobkov");
    QCoreApplication::setOrganizationDomain("alexander-bobkov.ru");
    QCoreApplication::setApplicationName("GeoPortal");

    QApplication app(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");
#if 1
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

    MainWindow appWin;

    osg::ref_ptr<osg::Node> node = osgDB::readNodeFile("data/globe.earth");
    osg::ref_ptr<osgEarth::MapNode> mapNode = osgEarth::MapNode::findMapNode(node);

    osgEarth::QtGui::ViewerWidget* viewerWidget = new osgEarth::QtGui::ViewerWidget(node);

    osgViewer::Viewer* viewer = dynamic_cast<osgViewer::Viewer*>(viewerWidget->getViewer());
    if (!viewer)
    {
        std::cerr << "Wrong viewer\n";
        return 1;
    }

    viewer->setThreadingModel(osgViewer::ViewerBase::SingleThreaded);

    EarthManipulator* earthManipulator = dynamic_cast<EarthManipulator*>(viewer->getCameraManipulator());
    earthManipulator->getSettings()->setScrollSensitivity(-1.0);

    osg::ref_ptr<osg::Uniform> highlightUniform = installHighlighter(mapNode->getOrCreateStateSet(), Registry::objectIndex()->getObjectIDAttribLocation());

    RTTPicker* picker = new RTTPicker();
    viewer->addEventHandler(picker);
    picker->addChild(mapNode);
    picker->setDefaultCallback(new MyPickCallback(highlightUniform, &appWin));
    
    DataManagerPtr dataManager = std::make_shared<DataManager>(viewer, mapNode);

    appWin.setCentralWidget(viewerWidget);
    appWin.setDataManager(dataManager);
    
    QSettings settings;
    QSize size = settings.value("MainWindow/size", QSize(1500, 900)).toSize();

    appWin.resize(size);
    appWin.show();

    int result = app.exec();
    return result;
}
