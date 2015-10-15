#include "MainWindow.hpp"
#include "MetadataWidget.hpp"

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
#include <QSettings>

using namespace osgEarth;
using namespace osgEarth::Drivers;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;

MainWindow::MainWindow() :
QMainWindow(),
_metadataDock(0)
{
    initUi();
}

MainWindow::~MainWindow()
{
}

void MainWindow::initUi()
{
    _ui.setupUi(this);

    connect(_ui.aboutAction, SIGNAL(triggered()), this, SLOT(showAbout()));
    connect(_ui.metadataAction, SIGNAL(triggered()), this, SLOT(showMetadataDescription()));

    connect(_ui.doQueryButton, SIGNAL(clicked()), this, SLOT(executeQuery()));

    _ui.dateTimeEditFrom->setDateTime(QDateTime::currentDateTime().addYears(-1));
    _ui.dateTimeEditTo->setDateTime(QDateTime::currentDateTime());
    
    _metadataDock = new QDockWidget(tr("Метаданные"));
    _metadataDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _metadataDock->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, _metadataDock);    

    MetadataWidget* metadataWidget = new MetadataWidget;
    connect(this, SIGNAL(sceneSelected(const QString&)), metadataWidget, SLOT(setSceneId(const QString&)));
    _metadataDock->setWidget(metadataWidget);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QSettings settings;
    settings.setValue("MainWindow/size", size());

    QMainWindow::resizeEvent(event);
}

void MainWindow::setMapNode(osgEarth::MapNode* mapNode)
{
    _mapNode = mapNode;
}

void MainWindow::setSceneId(const std::string& sceneid)
{
    if (sceneid.empty())
    {
        return;
    }

    _metadataDock->setVisible(true);

    emit sceneSelected(QString::fromUtf8(sceneid.c_str()));
}

void MainWindow::executeQuery()
{
    bool needAnd = false;

    std::ostringstream str;

    if (_ui.dateGroupBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }

        str << std::setprecision(7) << "scenetime>='" << _ui.dateTimeEditFrom->dateTime().toString(Qt::ISODate).toUtf8().constData() << "'::timestamp without time zone and " << "scenetime<='" << _ui.dateTimeEditTo->dateTime().toString(Qt::ISODate).toUtf8().constData() << "'::timestamp without time zone";
        needAnd = true;
    }

    if (_ui.sunAzimuthGroupBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }

        str << std::setprecision(7) << "sunazimuth>=" << _ui.sunAzimuthFromSpinBox->value() << " and " << "sunazimuth<=" << _ui.sunAzimuthToSpinBox->value();
        needAnd = true;
    }

    if (_ui.sunElevationGroupBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }

        str << std::setprecision(7) << "sunelevation>=" << _ui.sunElevationFromSpinBox->value() << " and " << "sunelevation<=" << _ui.sunElevationToSpinBox->value();
        needAnd = true;
    }

    if (_ui.inclinationGroupBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }

        str << std::setprecision(7) << "satelliteinclination>=" << _ui.inclinationFromSpinBox->value() << " and " << "satelliteinclination<=" << _ui.inclinationToSpinBox->value();
        needAnd = true;
    }

    if (_ui.lookAngleGroupBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }

        str << std::setprecision(7) << "lookangle>=" << _ui.lookAngleFromSpinBox->value() << " and " << "lookangle<=" << _ui.lookAngleToSpinBox->value();
        needAnd = true;
    }

    if (_ui.processingLevelGroupBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }

        if (_ui.l1RRadioButton->isChecked())
        {
            str << "processinglevel='L1R Product Available'";
        }
        else if (_ui.l1GstRadioButton->isChecked())
        {
            str << "processinglevel='L1Gst Product Available'";
        }
        else if (_ui.l1TRadioButton->isChecked())
        {
            str << "processinglevel='L1T Product Available'";
        }
        else
        {
            std::cerr << "Wrong processing level\n";
            return;
        }
        needAnd = true;
    }

    if (_ui.cloudnessCheckBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }
        
        str << "cloudmax<=" << _ui.cloudnessComboBox->currentText().toInt();
        needAnd = true;
    }
    
    if (_ui.orbitPathCheckBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }
        str << "orbitpath=" << _ui.orbitPathSpinBox->value();
        needAnd = true;
    }

    if (_ui.orbitRowCheckBox->isChecked())
    {        
        if (needAnd)
        {
            str << " and ";
        }
        str << "orbitrow=" << _ui.orbitRowSpinBox->value();
        needAnd = true;
    }
    
    if (_ui.targetPathCheckBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }
        str << "targetpath=" << _ui.targetPathSpinBox->value();
        needAnd = true;
    }

    if (_ui.targetRowCheckBox->isChecked())
    {
        if (needAnd)
        {
            str << " and ";
        }
        str << "targetrow=" << _ui.targetRowSpinBox->value();
        needAnd = true;
    }

    updateLayer(str.str());
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

    std::cout << "Query " << query << std::endl;

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
    poly->fill()->color() = Color::Yellow;

    LineSymbol* line = style.getOrCreate<LineSymbol>();
    line->stroke()->color() = Color::Yellow;
        
    AltitudeSymbol* alt = style.getOrCreate<AltitudeSymbol>();
    alt->clamping() = alt->CLAMP_TO_TERRAIN;
    alt->technique() = alt->TECHNIQUE_DRAPE;

    StyleSheet* styleSheet = new StyleSheet();
    styleSheet->addStyle(style);

    if (!query.empty())
    {
        Query q;
        q.expression() = query;

        osgEarth::Symbology::StyleSelector selector;
        selector.query() = q;
        styleSheet->selectors().push_back(selector);
    }

    //FeatureDisplayLayout layout;
    //layout.tileSizeFactor() = 52.0;
    //layout.addLevel(FeatureLevel(0.0f, 20000.0f, "buildings"));

    FeatureGeomModelOptions fgmOpt;
    fgmOpt.featureOptions() = featureOpt;
    fgmOpt.styles() = styleSheet;
    //fgmOpt.layout() = layout;

    _oldLayer = new ModelLayer("scenes", fgmOpt);
    _oldQuery = query;

    osg::Timer_t startTick = osg::Timer::instance()->tick();

    _mapNode->getMap()->addModelLayer(_oldLayer.get());    

    osg::Timer_t endTick = osg::Timer::instance()->tick();
    std::cout << "Loading time " << osg::Timer::instance()->delta_s(startTick, endTick) << std::endl;

    //int fc = dynamic_cast<osgEarth::Features::FeatureModelSource*>(_oldLayer->getModelSource())->getFeatureSource()->getFeatureCount();
    //std::cout << "Count = " << fc << std::endl;
}

void MainWindow::showAbout()
{
    QDialog dialog(this);

    dialog.setWindowTitle(tr("О программе"));

    QVBoxLayout* vLayout = new QVBoxLayout;
    dialog.setLayout(vLayout);

    QString text = QString::fromUtf8("<html><head/><body><p align='center'><span style='font-size:12pt;'>Геопортал</span></p><p>Разработчики:<br/>Александр Бобков<br/>Денис Учаев</p></body></html>");

    QLabel* aboutLabel = new QLabel(text);
    aboutLabel->setTextFormat(Qt::RichText);
    aboutLabel->setOpenExternalLinks(true);
    vLayout->addWidget(aboutLabel);

    QHBoxLayout* hLayout = new QHBoxLayout;
    hLayout->addStretch();

    QPushButton* okButton = new QPushButton("OK");
    hLayout->addWidget(okButton);

    vLayout->addLayout(hLayout);

    connect(okButton, SIGNAL(clicked()), &dialog, SLOT(accept()));

    dialog.exec();
}

void MainWindow::showMetadataDescription()
{
    QDesktopServices::openUrl(QUrl("https://lta.cr.usgs.gov/EO1.html"));
}