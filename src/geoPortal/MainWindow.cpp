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

    connect(_ui.pushButton, SIGNAL(clicked()), this, SLOT(loadScenes()));
}

void MainWindow::setMapNode(osgEarth::MapNode* mapNode)
{
    _mapNode = mapNode;
}

void MainWindow::loadScenes()
{
    std::cout << "Load scenes\n";

    OGRFeatureOptions featureOpt;
    featureOpt.ogrDriver() = "PostgreSQL";
    //featureOpt.connection() = "PG:dbname='GeoPortal' host='178.62.140.44' port='5432' user='portal' password='PortalPass'";
    featureOpt.connection() = "PG:dbname='GeoPortal' host='localhost' port='5432' user='user' password='user'";
    featureOpt.layer() = "scenes";
    //featureOpt.buildSpatialIndex() = true;

    //PG:"dbname='databasename' host='addr' port='5432' user='x' password='y'"

    Style style;

    PolygonSymbol* poly = style.getOrCreate<PolygonSymbol>();
    poly->fill()->color() = Color::White;
        
    AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;
    alt->technique() = alt->TECHNIQUE_DRAPE;

    StyleSheet* styleSheet = new StyleSheet();
    styleSheet->addStyle(style);

    //FeatureDisplayLayout layout;
    //layout.tileSizeFactor() = 52.0;
    //layout.addLevel(FeatureLevel(0.0f, 20000.0f, "buildings"));

    FeatureGeomModelOptions fgmOpt;
    fgmOpt.featureOptions() = featureOpt;
    fgmOpt.styles() = styleSheet;
    //fgmOpt.layout() = layout;

    _mapNode->getMap()->addModelLayer(new ModelLayer("scenes", fgmOpt));

    _ui.pushButton->setEnabled(false);
}