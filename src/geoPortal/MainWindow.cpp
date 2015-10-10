#include "MainWindow.hpp"

#include <osgEarthFeatures/FeatureSource>
#include <osgEarthFeatures/FeatureDisplayLayout>
#include <osgEarthSymbology/StyleSheet>
#include <osgEarthSymbology/PolygonSymbol>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>

#include <QAction>
#include <QDockWidget>
#include <QDesktopServices>
#include <QLocale>
#include <QtGui>
#include <QMainWindow>
#include <QToolBar>
#include <QWidgetAction>
#include <QFileDialog>
#include <QProgressDialog>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>

using namespace osgEarth;
using namespace osgEarth::Drivers;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;

MainWindow::MainWindow() :
QMainWindow()
{
    initUi();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUi()
{
    _ui.setupUi(this);

    connect(_ui.pushButton, SIGNAL(clicked()), this, SLOT(executeQuery()));
}

void MainWindow::setMapNode(osgEarth::MapNode* mapNode)
{
    _mapNode = mapNode;
}

void MainWindow::executeQuery()
{
    updateLayer("orbitpath=10");
}

void MainWindow::updateLayer(const std::string& query)
{
    if (_oldLayer.valid() && query == _oldQuery)
    {
        return;
    }

    if (_oldLayer.valid())
    {
        _mapNode->getMap()->removeModelLayer(_oldLayer.get());
        _oldLayer = nullptr;
    }

    OGRFeatureOptions featureOpt;
    featureOpt.ogrDriver() = "PostgreSQL";
#if 0
    featureOpt.connection() = "PG:dbname='GeoPortal' host='178.62.140.44' port='5432' user='portal' password='PortalPass'";
#else
    featureOpt.connection() = "PG:dbname='GeoPortal' host='localhost' port='5432' user='user' password='user'";
#endif
    featureOpt.layer() = "scenes";
    featureOpt.buildSpatialIndex() = true;

    Style style;

    PolygonSymbol* poly = style.getOrCreate<PolygonSymbol>();
    poly->fill()->color() = Color::Green;

    LineSymbol* line = style.getOrCreate<LineSymbol>();
    line->stroke()->color() = Color::Yellow;
        
    AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;
    alt->technique() = alt->TECHNIQUE_DRAPE;

    StyleSheet* styleSheet = new StyleSheet();
    styleSheet->addStyle(style);

    Query q;
    q.expression() = query;

    osgEarth::Symbology::StyleSelector selector;
    selector.query() = q;
    styleSheet->selectors().push_back(selector);

    //FeatureDisplayLayout layout;
    //layout.tileSizeFactor() = 52.0;
    //layout.addLevel(FeatureLevel(0.0f, 20000.0f, "buildings"));

    FeatureGeomModelOptions fgmOpt;
    fgmOpt.featureOptions() = featureOpt;
    fgmOpt.styles() = styleSheet;
    //fgmOpt.layout() = layout;

    _oldLayer = new ModelLayer("scenes", fgmOpt);
    _oldQuery = query;

    _mapNode->getMap()->addModelLayer(_oldLayer.get());    

    int fc = dynamic_cast<osgEarth::Features::FeatureModelSource*>(_oldLayer->getModelSource())->getFeatureSource()->getFeatureCount();
    std::cout << "Count = " << fc << std::endl;
}