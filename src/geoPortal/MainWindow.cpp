#include "MainWindow.hpp"
#include "MetadataWidget.hpp"
#include "Dataset.hpp"
#include "TableModel.hpp"
#include "ProxyModel.hpp"
#include "SettingsWidget.hpp"
#include "SceneOperationsWidget.hpp"

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
#include <QtConcurrent/QtConcurrent>

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
_progressBar(0),
_sceneWidgetDock(0),
_scenesMainDock(0),
_scenesMainView(0),
_scenesSecondDock(0),
_scenesSecondView(0),
_downloadManager(0)
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
    connect(_ui.settingsAction, SIGNAL(triggered()), this, SLOT(showSettings()));

    connect(_ui.doQueryButton, SIGNAL(clicked()), this, SLOT(executeQuery()));

    //--------------------------------------------

    QSettings settings;

    _ui.dateGroupBox->setChecked(settings.value("Query/dateEnabled", false).toBool());
    _ui.dateTimeEditFrom->setDateTime(settings.value("Query/dateFrom", QDateTime::currentDateTime().addYears(-1)).toDateTime());
    _ui.dateTimeEditTo->setDateTime(settings.value("Query/dateTo", QDateTime::currentDateTime()).toDateTime());

    _ui.sunAzimuthGroupBox->setChecked(settings.value("Query/sunAzimuthEnabled", false).toBool());
    _ui.sunAzimuthFromSpinBox->setValue(settings.value("Query/sunAzimuthFrom").toDouble());
    _ui.sunAzimuthToSpinBox->setValue(settings.value("Query/sunAzimuthTo").toDouble());

    _ui.sunElevationGroupBox->setChecked(settings.value("Query/sunElevationEnabled", false).toBool());
    _ui.sunElevationFromSpinBox->setValue(settings.value("Query/sunElevationFrom").toDouble());
    _ui.sunElevationToSpinBox->setValue(settings.value("Query/sunElevationTo").toDouble());

    _ui.inclinationGroupBox->setChecked(settings.value("Query/inclinationEnabled", false).toBool());
    _ui.inclinationFromSpinBox->setValue(settings.value("Query/inclinationFrom").toDouble());
    _ui.inclinationToSpinBox->setValue(settings.value("Query/inclinationTo").toDouble());

    _ui.lookAngleGroupBox->setChecked(settings.value("Query/lookAngleEnabled", false).toBool());
    _ui.lookAngleFromSpinBox->setValue(settings.value("Query/lookAngleFrom").toDouble());
    _ui.lookAngleToSpinBox->setValue(settings.value("Query/lookAngleTo").toDouble());

    _ui.processingLevelGroupBox->setChecked(settings.value("Query/processingLevelEnabled", false).toBool());
    int processingLevelValue = settings.value("Query/processingLevelValue", 2).toInt();
    if (processingLevelValue == 0)
    {
        _ui.l1RRadioButton->setChecked(true);
    }
    else if(processingLevelValue == 1)
    {
        _ui.l1GstRadioButton->setChecked(true);
    }
    else if (processingLevelValue == 2)
    {
        _ui.l1TRadioButton->setChecked(true);
    }

    _ui.cloudnessCheckBox->setChecked(settings.value("Query/cloudnessEnabled", false).toBool());
    _ui.cloudnessComboBox->setCurrentText(QString::number(settings.value("Query/cloudnessValue").toInt()));

    _ui.orbitPathCheckBox->setChecked(settings.value("Query/orbitPathEnabled", false).toBool());
    _ui.orbitPathSpinBox->setValue(settings.value("Query/orbitPathValue").toInt());

    _ui.orbitRowCheckBox->setChecked(settings.value("Query/orbitRowEnabled", false).toBool());
    _ui.orbitRowSpinBox->setValue(settings.value("Query/orbitRowValue").toInt());

    _ui.targetPathCheckBox->setChecked(settings.value("Query/targetPathEnabled", false).toBool());
    _ui.targetPathSpinBox->setValue(settings.value("Query/targetPathValue").toInt());

    _ui.targetRowCheckBox->setChecked(settings.value("Query/targetRowEnabled", false).toBool());
    _ui.targetRowSpinBox->setValue(settings.value("Query/targetRowValue").toInt());

    _ui.distanceGroupBox->setChecked(settings.value("Query/distanceEnabled", false).toBool());
    _ui.longitudeSpinBox->setValue(settings.value("Query/centerLongitude").toDouble());
    _ui.latitudeSpinBox->setValue(settings.value("Query/centerLatitude").toDouble());
    _ui.distanceSpinBox->setValue(settings.value("Query/distanceValue", 1000.0).toDouble());
    
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

    _sceneWidgetDock = new QDockWidget(tr("Выбранная сцена"));
    _sceneWidgetDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    _sceneWidgetDock->setVisible(false);
    addDockWidget(Qt::RightDockWidgetArea, _sceneWidgetDock, Qt::Horizontal);

    //--------------------------------------------

    _progressBar = new QProgressBar(this);
    _progressBar->setMaximumWidth(200);
    _progressBar->setMinimum(0);
    _progressBar->setMaximum(100);
    _progressBar->setTextVisible(false);
    statusBar()->addWidget(_progressBar);    
}

void MainWindow::moveEvent(QMoveEvent* event)
{
    QSettings settings;
    settings.setValue("MainWindow/pos", pos());

    QMainWindow::moveEvent(event);
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

    _handler = new SelectPointMouseHandler(_dataManager->mapNode(),
                                           std::bind(&MainWindow::onMousePositionChanged, this, std::placeholders::_1),
                                           std::bind(&MainWindow::onMouseClicked, this));

    _dataManager->view()->addEventHandler(_handler);

    //--------------------------------------------

    QWidget* sceneWidget = new QWidget(this);
    QVBoxLayout* layout = new QVBoxLayout(sceneWidget);
    sceneWidget->setLayout(layout);

    MetadataWidget* metadataWidget = new MetadataWidget(_dataManager, this);
    connect(this, SIGNAL(sceneSelected(const ScenePtr&)), metadataWidget, SLOT(setScene(const ScenePtr&)));    
    layout->addWidget(metadataWidget);    
            
    SceneOperationsWidget* sceneOperationsWidget = new SceneOperationsWidget(_dataManager, this);    
    connect(this, SIGNAL(sceneSelected(const ScenePtr&)), sceneOperationsWidget, SLOT(setScene(const ScenePtr&)));
    layout->addWidget(sceneOperationsWidget);

    layout->addStretch();
    
    _sceneWidgetDock->setWidget(sceneWidget);

    //--------------------------------------------

    _downloadManager = new DownloadManager(_dataManager, this);
    connect(this, SIGNAL(sceneSelected(const ScenePtr&)), _downloadManager, SLOT(downloadOverview(const ScenePtr&)));
}

void MainWindow::setScene(const ScenePtr& scene)
{
    if (!scene)
    {
        return;
    }

    _sceneWidgetDock->setVisible(true);

    emit sceneSelected(scene);
}

void MainWindow::executeQuery()
{
    //Сначала подчистим старое

    _ui.dockWidget->setEnabled(false);
    _progressBar->setMaximum(0);

    _dataManager->setDataSet(DataSetPtr());

    QAbstractItemModel *oldMainModel = _scenesMainView->model();
    if (oldMainModel)
    {
        QItemSelectionModel* selectionModel = _scenesMainView->selectionModel();
        _scenesMainView->setModel(0);
        delete selectionModel;
        delete oldMainModel;
    }

    QAbstractItemModel *oldSecondModel = _scenesSecondView->model();
    if (oldSecondModel)
    {
        QItemSelectionModel* selectionModel = _scenesSecondView->selectionModel();
        _scenesSecondView->setModel(0);
        delete selectionModel;
        delete oldSecondModel;
    }

    //-----------------------------------------------------

    QSettings settings;

    settings.setValue("Query/dateEnabled", _ui.dateGroupBox->isChecked());
    settings.setValue("Query/dateFrom", _ui.dateTimeEditFrom->dateTime());
    settings.setValue("Query/dateTo", _ui.dateTimeEditTo->dateTime());

    settings.setValue("Query/sunAzimuthEnabled", _ui.sunAzimuthGroupBox->isChecked());
    settings.setValue("Query/sunAzimuthFrom", _ui.sunAzimuthFromSpinBox->value());
    settings.setValue("Query/sunAzimuthTo", _ui.sunAzimuthToSpinBox->value());

    settings.setValue("Query/sunElevationEnabled", _ui.sunElevationGroupBox->isChecked());
    settings.setValue("Query/sunElevationFrom", _ui.sunElevationFromSpinBox->value());
    settings.setValue("Query/sunElevationTo", _ui.sunElevationFromSpinBox->value());

    settings.setValue("Query/inclinationEnabled", _ui.inclinationGroupBox->isChecked());
    settings.setValue("Query/inclinationFrom", _ui.inclinationFromSpinBox->value());
    settings.setValue("Query/inclinationTo", _ui.inclinationToSpinBox->value());

    settings.setValue("Query/lookAngleEnabled", _ui.lookAngleGroupBox->isChecked());
    settings.setValue("Query/lookAngleFrom", _ui.lookAngleFromSpinBox->value());
    settings.setValue("Query/lookAngleTo", _ui.lookAngleToSpinBox->value());

    settings.setValue("Query/processingLevelEnabled", _ui.processingLevelGroupBox->isChecked());
    if (_ui.l1RRadioButton->isChecked())
    {
        settings.setValue("Query/processingLevelValue", 0);
    }
    else if (_ui.l1GstRadioButton->isChecked())
    {
        settings.setValue("Query/processingLevelValue", 1);
    }
    else if (_ui.l1TRadioButton->isChecked())
    {
        settings.setValue("Query/processingLevelValue", 2);
    }
    else
    {
        qDebug() << "Wrong processing level";
    }

    settings.setValue("Query/cloudnessEnabled", _ui.cloudnessCheckBox->isChecked());
    settings.setValue("Query/cloudnessValue", _ui.cloudnessComboBox->currentText().toInt());

    settings.setValue("Query/orbitPathEnabled", _ui.orbitPathCheckBox->isChecked());
    settings.setValue("Query/orbitPathValue", _ui.orbitPathSpinBox->value());

    settings.setValue("Query/orbitRowEnabled", _ui.orbitRowCheckBox->isChecked());
    settings.setValue("Query/orbitRowValue", _ui.orbitRowSpinBox->value());

    settings.setValue("Query/targetPathEnabled", _ui.targetPathCheckBox->isChecked());
    settings.setValue("Query/targetPathValue", _ui.targetPathSpinBox->value());

    settings.setValue("Query/targetRowEnabled", _ui.targetRowCheckBox->isChecked());
    settings.setValue("Query/targetRowValue", _ui.targetRowSpinBox->value());

    settings.setValue("Query/distanceEnabled", _ui.distanceGroupBox->isChecked());
    settings.setValue("Query/centerLongitude", _ui.longitudeSpinBox->value());
    settings.setValue("Query/centerLatitude", _ui.latitudeSpinBox->value());
    settings.setValue("Query/distanceValue", _ui.distanceSpinBox->value());

    //-----------------------------------------------------    

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
            qDebug() << "Wrong processing level";
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
    
    QtConcurrent::run(this, &MainWindow::loadScenes);
}

void MainWindow::loadScenes()
{
    _dataset->selectScenes();

    _dataManager->setDataSet(_dataset);

    QMetaObject::invokeMethod(this, "finishLoadScenes", Qt::QueuedConnection);
}

void MainWindow::finishLoadScenes()
{
    TableModel* tableModel = new TableModel(_dataset, this);
    _scenesMainView->setModel(tableModel);
    _scenesMainView->resizeColumnsToContents();

    _scenesMainDock->setVisible(true);
    _scenesSecondDock->setVisible(true);

    connect(_scenesMainView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onMainTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));

    _progressBar->setMaximum(100);
    _ui.dockWidget->setEnabled(true);
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

void MainWindow::showSettings()
{
    SettingsWidget* widget = new SettingsWidget(_dataManager, this);
    widget->setWindowFlags(Qt::Window);
    widget->setAttribute(Qt::WA_DeleteOnClose);
    widget->show();
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
        if (_dataset && _dataset->isInitialized())
        {
            _dataset->selectScenesUnderPointer(_point);
            
            ProxyModel* proxyModel = new ProxyModel(_dataset, this);
            proxyModel->setSourceModel(_scenesMainView->model());

            _scenesSecondView->setModel(proxyModel);            
            _scenesSecondView->resizeColumnsToContents();

            connect(_scenesSecondView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onSecondTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));

            if (proxyModel->rowCount() > 0)
            {
                ScenePtr scene = proxyModel->data(proxyModel->index(0, 0), Qt::UserRole).value<ScenePtr>();
                setScene(scene);
            }
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
    if (!proxyModel)
    {
        return;
    }

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