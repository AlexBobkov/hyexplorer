#include "MainWindow.hpp"
#include "MetadataWidget.hpp"
#include "Dataset.hpp"
#include "TableModel.hpp"
#include "ProxyModel.hpp"

#include <osgEarth/Terrain>
#include <osgEarthAnnotation/CircleNode>

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

#include <functional>

using namespace osgEarth;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace portal;

namespace
{
    struct SelectPointMouseHandler : public osgGA::GUIEventHandler
    {
        typedef std::function<void(const osgEarth::GeoPoint& point)> PointCallbackType;
        typedef std::function<void()> FinishCallbackType;

        SelectPointMouseHandler(osgEarth::MapNode* mapNode, const PointCallbackType &pcb, const FinishCallbackType& fcb) :
            osgGA::GUIEventHandler(),
            _mapNode(mapNode),
            _pointCB(pcb),
            _finishCB(fcb),
            _mouseX(0.0),
            _mouseY(0.0)
        {
        }

        ~SelectPointMouseHandler()
        {
        }

        bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
        {
            osgViewer::View* view = static_cast<osgViewer::View*>(aa.asView());

            if (ea.getEventType() == osgGA::GUIEventAdapter::MOVE ||
                (ea.getEventType() == osgGA::GUIEventAdapter::PUSH && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON) ||
                (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON))
            {
                osg::Vec3d world;
                if (_mapNode->getTerrain()->getWorldCoordsUnderMouse(aa.asView(), ea.getX(), ea.getY(), world))
                {
                    osgEarth::GeoPoint mapPoint;
                    mapPoint.fromWorld(_mapNode->getMapSRS(), world);

                    _pointCB(mapPoint);
                }

                if (ea.getEventType() == osgGA::GUIEventAdapter::PUSH && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
                {
                    _mouseX = ea.getX();
                    _mouseY = ea.getY();
                }

                if (ea.getEventType() == osgGA::GUIEventAdapter::RELEASE && ea.getButton() == osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON)
                {
                    if (fabs(ea.getX() - _mouseX) < 2.0f && fabs(ea.getY() - _mouseY) < 2.0f)
                    {
                        _finishCB();
                    }
                }
            }

            return false;
        }

        osg::observer_ptr<MapNode>  _mapNode;
        PointCallbackType _pointCB;
        FinishCallbackType _finishCB;

        float _mouseX;
        float _mouseY;
    };
}

MainWindow::MainWindow() :
QMainWindow(),
_metadataDock(0),
_scenesMainDock(0),
_scenesMainView(0),
_scenesSecondDock(0),
_scenesSecondView(0)
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

    //--------------------------------------------
        
    _scenesMainDock = new QDockWidget(tr("Найденные сцены"));
    _scenesMainDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //_scenesDock->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, _scenesMainDock);

    _scenesMainView = new QTableView(this);
    _scenesMainDock->setWidget(_scenesMainView);

    connect(_scenesMainView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectScene(const QModelIndex&)));
    connect(_scenesMainView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(zoomToScene(const QModelIndex&)));
    
    //--------------------------------------------

    _scenesSecondDock = new QDockWidget(tr("Сцены под указателем"));
    _scenesSecondDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    //_scenes2Dock->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, _scenesSecondDock);

    _scenesSecondView = new QTableView(this);
    _scenesSecondDock->setWidget(_scenesSecondView);

    connect(_scenesSecondView, SIGNAL(clicked(const QModelIndex&)), this, SLOT(selectScene(const QModelIndex&)));
    connect(_scenesSecondView, SIGNAL(doubleClicked(const QModelIndex&)), this, SLOT(zoomToScene(const QModelIndex&)));
    
    //--------------------------------------------

    _metadataDock = new QDockWidget(tr("Метаданные"));
    _metadataDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _metadataDock->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, _metadataDock, Qt::Horizontal);
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QSettings settings;
    settings.setValue("MainWindow/size", size());

    QMainWindow::resizeEvent(event);
}

void MainWindow::setDataManager(const DataManagerPtr& dataManager)
{
    _dataManager = dataManager;

    MetadataWidget* metadataWidget = new MetadataWidget;
    connect(this, SIGNAL(sceneSelected(const ScenePtr&)), metadataWidget, SLOT(setScene(const ScenePtr&)));
    metadataWidget->setMapNode(_dataManager->mapNode());
    _metadataDock->setWidget(metadataWidget);

    _handler = new SelectPointMouseHandler(_dataManager->mapNode(),
                                           std::bind(&MainWindow::onMousePositionChanged, this, std::placeholders::_1),
                                           std::bind(&MainWindow::onMouseClicked, this));

    _dataManager->view()->addEventHandler(_handler);
}

void MainWindow::setScene(const ScenePtr& scene)
{
    if (!scene)
    {
        return;
    }

    _metadataDock->setVisible(true);

    emit sceneSelected(scene);
}

void MainWindow::executeQuery()
{
    bool needAnd = false;

    std::ostringstream str;

    _dataset = std::make_shared<DataSet>();

    if (_ui.dateGroupBox->isChecked())
    {
        _dataset->addCondition(QString("scenetime>='%0'::timestamp without time zone and scenetime<='%1'::timestamp without time zone").arg(_ui.dateTimeEditFrom->dateTime().toString(Qt::ISODate)).arg(_ui.dateTimeEditTo->dateTime().toString(Qt::ISODate)));
    }

    if (_ui.sunAzimuthGroupBox->isChecked())
    {
        _dataset->addCondition(QString("sunazimuth>=%0 and sunazimuth<=%1").arg(_ui.sunAzimuthFromSpinBox->value(), 0, 'f', 7).arg(_ui.sunAzimuthToSpinBox->value(), 0, 'f', 7));
    }

    if (_ui.sunElevationGroupBox->isChecked())
    {
        _dataset->addCondition(QString("sunelevation>=%0 and sunelevation<=%1").arg(_ui.sunElevationFromSpinBox->value(), 0, 'f', 7).arg(_ui.sunElevationToSpinBox->value(), 0, 'f', 7));
    }

    if (_ui.inclinationGroupBox->isChecked())
    {
        _dataset->addCondition(QString("satelliteinclination>=%0 and satelliteinclination<=%1").arg(_ui.inclinationFromSpinBox->value(), 0, 'f', 7).arg(_ui.inclinationToSpinBox->value(), 0, 'f', 7));
    }

    if (_ui.lookAngleGroupBox->isChecked())
    {
        _dataset->addCondition(QString("lookangle>=%0 and lookangle<=%1").arg(_ui.lookAngleFromSpinBox->value(), 0, 'f', 7).arg(_ui.lookAngleToSpinBox->value(), 0, 'f', 7));
    }

    if (_ui.processingLevelGroupBox->isChecked())
    {
        if (_ui.l1RRadioButton->isChecked())
        {
            _dataset->addCondition("processinglevel='L1R Product Available'");
        }
        else if (_ui.l1GstRadioButton->isChecked())
        {
            _dataset->addCondition("processinglevel='L1Gst Product Available'");
        }
        else if (_ui.l1TRadioButton->isChecked())
        {
            _dataset->addCondition("processinglevel='L1T Product Available'");
        }
        else
        {
            std::cerr << "Wrong processing level\n";
        }
    }

    if (_ui.cloudnessCheckBox->isChecked())
    {
        _dataset->addCondition(QString("cloudmax<=%0").arg(_ui.cloudnessComboBox->currentText().toInt()));
    }

    if (_ui.orbitPathCheckBox->isChecked())
    {
        _dataset->addCondition(QString("orbitpath=%0").arg(_ui.orbitPathSpinBox->value()));
    }

    if (_ui.orbitRowCheckBox->isChecked())
    {
        _dataset->addCondition(QString("orbitrow=%0").arg(_ui.orbitRowSpinBox->value()));
    }

    if (_ui.targetPathCheckBox->isChecked())
    {
        _dataset->addCondition(QString("targetpath=%0").arg(_ui.targetPathSpinBox->value()));
    }

    if (_ui.targetRowCheckBox->isChecked())
    {
        _dataset->addCondition(QString("targetrow=%0").arg(_ui.targetRowSpinBox->value()));
    }

    if (_ui.distanceGroupBox->isChecked())
    {
        _dataset->addCondition(QString("ST_DWithin(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'),%3)").arg(_ui.longitudeSpinBox->value(), 0, 'f', 12).arg(_ui.latitudeSpinBox->value(), 0, 'f', 12).arg(_ui.distanceSpinBox->value() * 1000));
                
        _dataManager->setCircleNode(GeoPoint(_dataManager->mapNode()->getMapSRS(), _ui.longitudeSpinBox->value(), _ui.latitudeSpinBox->value(), 0.0, osgEarth::ALTMODE_ABSOLUTE), _ui.distanceSpinBox->value() * 1000);
    }
    else
    {
        _dataManager->removeCircleNode();
    }

    _dataset->selectScenes();

    _dataManager->setDataSet(_dataset);

    TableModel* tableModel = new TableModel(_dataset, this);    
    _scenesMainView->setModel(tableModel);    
    _scenesMainView->resizeColumnsToContents();
    _scenesMainDock->setVisible(true);
    
    _scenesSecondDock->setVisible(true);    

    connect(_scenesMainView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onMainTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));
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

void MainWindow::onMousePositionChanged(const osgEarth::GeoPoint& point)
{
    if (_ui.selectPointButton->isChecked())
    {
        _ui.longitudeSpinBox->setValue(point.x());
        _ui.latitudeSpinBox->setValue(point.y());
    }
    else
    {
        _point = point;
    }
}

void MainWindow::onMouseClicked()
{
    if (_ui.selectPointButton->isChecked())
    {
        _ui.selectPointButton->setChecked(false);
    }
    else
    {
        if (_dataset)
        {
            _dataset->selectScenesUnderPointer(_point);
            
            ProxyModel* proxyModel = new ProxyModel(_dataset, this);
            proxyModel->setSourceModel(_scenesMainView->model());

            _scenesSecondView->setModel(proxyModel);            
            _scenesSecondView->resizeColumnsToContents();

            connect(_scenesSecondView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onSecondTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));
        }
    }
}

void MainWindow::selectScene(const QModelIndex& index)
{
    ScenePtr scene = index.data(Qt::UserRole).value<ScenePtr>();
    setScene(scene);
}

void MainWindow::zoomToScene(const QModelIndex& index)
{
    ScenePtr scene = index.data(Qt::UserRole).value<ScenePtr>();
    _dataManager->zoomToScene(scene);
}

void MainWindow::onMainTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    ProxyModel* proxyModel = dynamic_cast<ProxyModel*>(_scenesSecondView->model());
    QItemSelection proxySelection = proxyModel->mapSelectionFromSource(selected);

    _scenesSecondView->selectionModel()->select(proxySelection, QItemSelectionModel::ClearAndSelect);
    if (!proxySelection.empty())
    {
        _scenesSecondView->scrollTo(proxySelection.first().indexes()[0]);
    }
}

void MainWindow::onSecondTableViewSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    ProxyModel* proxyModel = dynamic_cast<ProxyModel*>(_scenesSecondView->model());
    QItemSelection sourceSelection = proxyModel->mapSelectionToSource(selected);

    _scenesMainView->selectionModel()->select(sourceSelection, QItemSelectionModel::ClearAndSelect);
    if (!sourceSelection.empty())
    {
        _scenesMainView->scrollTo(sourceSelection.first().indexes()[0]);
    }
}