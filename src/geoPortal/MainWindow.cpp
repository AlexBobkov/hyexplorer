#include "MainWindow.hpp"
#include "MetadataWidget.hpp"
#include "Dataset.hpp"
#include "TableModel.hpp"
#include "ProcessingWidget.hpp"
#include "ProxyModel.hpp"
#include "SettingsWidget.hpp"
#include "SceneOperationsWidget.hpp"
#include "Operations.hpp"
#include "Version.hpp"
#include "EventHandlers.hpp"

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
using namespace portal;

MainWindow::MainWindow(const DataManagerPtr& dataManager) :
QMainWindow(),
_dataManager(dataManager),
_progressBar(0),
_scenesMainView(0),
_scenesSecondView(0),
_mousePosLabel(0)
{
    initUi();
}

MainWindow::~MainWindow()
{
    _dataManager->removeReportHandler(_mouseReportHandler);
    _dataManager->setDefaultActionHandler(0);
    _dataManager->setActionHandler(0);
}

void MainWindow::initUi()
{
    _ui.setupUi(this);

    connect(_ui.aboutAction, &QAction::triggered, this, [this]()
    {
        QDialog dialog(this);

        dialog.setWindowTitle(tr("О программе"));

        QVBoxLayout* vLayout = new QVBoxLayout;
        dialog.setLayout(vLayout);

        QString text = tr("<html><head/><body>"
                          "<p align='center'><span style='font-size:12pt;'>HyExplorer</span></p>"
                          "<p>Версия: %0. Время сборки %1 %2</p>"
                          "<p>Разработчики:<br/><a href='mailto:alexander.e.bobkov@gmail.com'>Александр Бобков</a><br/><a href='mailto:d-uchaev@ya.ru'>Денис Учаев</a></p>"
                          "<p>Разработка поддержана грантом РФФИ №13-05-12086</p>"
                          "</body></html>").arg(Version::str).arg(__DATE__).arg(__TIME__);

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
    });

    connect(_ui.settingsAction, &QAction::triggered, this, [this]()
    {
        SettingsWidget* widget = new SettingsWidget(_dataManager, this);
        widget->setWindowFlags(Qt::Window);
        widget->setAttribute(Qt::WA_DeleteOnClose);
        widget->show();
    });

    connect(_ui.metadataHyperionAction, &QAction::triggered, this, [this]()
    {
        QDesktopServices::openUrl(QUrl("https://lta.cr.usgs.gov/EO1.html"));
    });

    connect(_ui.dataAvirisAction, &QAction::triggered, this, [this]()
    {
        QDesktopServices::openUrl(QUrl("http://aviris.jpl.nasa.gov/alt_locator/111013_AV_Download.readme"));
    });

    connect(_ui.doQueryButton, &QPushButton::clicked, this, &MainWindow::executeQuery);

    auto sensorChangedCB = [this](bool)
    {
        _ui.commonGroupBox->setVisible(_ui.hyperionCheckBox->isChecked() || _ui.avirisCheckBox->isChecked());
        _ui.hyperionGroupBox->setVisible(_ui.hyperionCheckBox->isChecked() && !_ui.avirisCheckBox->isChecked());
        _ui.avirisGroupBox->setVisible(!_ui.hyperionCheckBox->isChecked() && _ui.avirisCheckBox->isChecked());
    };

    connect(_ui.hyperionCheckBox, &QCheckBox::toggled, this, sensorChangedCB);
    connect(_ui.avirisCheckBox, &QCheckBox::toggled, this, sensorChangedCB);

    sensorChangedCB(true);

    //--------------------------------------------

    QSettings settings;

    _ui.hyperionCheckBox->setChecked(settings.value("Query/hyperionChecked", true).toBool());
    _ui.avirisCheckBox->setChecked(settings.value("Query/avirisChecked", false).toBool());

    _ui.sceneIdCheckBox->setChecked(settings.value("Query/sceneIdEnabled", false).toBool());
    _ui.sceneIdLineEdit->setText(settings.value("Query/sceneIdText", QString()).toString());

    _ui.hasSceneCheckBox->setChecked(settings.value("Query/hasSceneEnabled", false).toBool());

    _ui.dateGroupBox->setChecked(settings.value("Query/dateEnabled", false).toBool());
    _ui.dateTimeEditFrom->setDateTime(settings.value("Query/dateFrom", QDateTime::currentDateTime().addYears(-1)).toDateTime());
    _ui.dateTimeEditTo->setDateTime(settings.value("Query/dateTo", QDateTime::currentDateTime()).toDateTime());

    _ui.sunAzimuthGroupBox->setChecked(settings.value("Query/sunAzimuthEnabled", false).toBool());
    _ui.sunAzimuthFromSpinBox->setValue(settings.value("Query/sunAzimuthFrom").toDouble());
    _ui.sunAzimuthToSpinBox->setValue(settings.value("Query/sunAzimuthTo").toDouble());

    _ui.sunElevationGroupBox->setChecked(settings.value("Query/sunElevationEnabled", false).toBool());
    _ui.sunElevationFromSpinBox->setValue(settings.value("Query/sunElevationFrom").toDouble());
    _ui.sunElevationToSpinBox->setValue(settings.value("Query/sunElevationTo").toDouble());

    _ui.pixelSizeGroupBox->setChecked(settings.value("Query/pixelSizeEnabled", false).toBool());
    _ui.pixelSizeFromSpinBox->setValue(settings.value("Query/pixelSizeFrom").toDouble());
    _ui.pixelSizeToSpinBox->setValue(settings.value("Query/pixelSizeTo").toDouble());

    _ui.distanceGroupBox->setChecked(settings.value("Query/distanceEnabled", false).toBool());
    _ui.longitudeSpinBox->setValue(settings.value("Query/centerLongitude").toDouble());
    _ui.latitudeSpinBox->setValue(settings.value("Query/centerLatitude").toDouble());
    _ui.distanceSpinBox->setValue(settings.value("Query/distanceValue", 1000.0).toDouble());

    //-- Hyperion

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
    else if (processingLevelValue == 1)
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

    //-- AVIRIS

    _ui.siteNameCheckBox->setChecked(settings.value("Query/siteNameEnabled", false).toBool());
    _ui.siteNameLineEdit->setText(settings.value("Query/siteNameText", QString()).toString());

    _ui.rotationGroupBox->setChecked(settings.value("Query/rotationEnabled", false).toBool());
    _ui.rotationFromSpinBox->setValue(settings.value("Query/rotationFrom").toDouble());
    _ui.rotationToSpinBox->setValue(settings.value("Query/rotationTo").toDouble());

    _ui.meanElevationGroupBox->setChecked(settings.value("Query/meanElevationEnabled", false).toBool());
    _ui.meanElevationFromSpinBox->setValue(settings.value("Query/meanElevationFrom").toDouble());
    _ui.meanElevationToSpinBox->setValue(settings.value("Query/meanElevationTo").toDouble());

    //--

    _ui.toolsMenu->addAction(_ui.dockWidget->toggleViewAction());

    //--------------------------------------------

    QDockWidget* scenesMainDock = new QDockWidget(tr("Результаты поиска"));
    scenesMainDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, scenesMainDock, Qt::Vertical);

    _scenesMainView = new QTableView(this);
    scenesMainDock->setWidget(_scenesMainView);

    connect(_scenesMainView, &QTableView::clicked, this, [this](const QModelIndex& index)
    {
        ScenePtr scene = index.data(Qt::UserRole).value<ScenePtr>();
        emit sceneSelected(scene);
    });
    connect(_scenesMainView, &QTableView::doubleClicked, this, [this](const QModelIndex& index)
    {
        ScenePtr scene = index.data(Qt::UserRole).value<ScenePtr>();
        _dataManager->zoomToScene(scene);
    });

    _ui.toolsMenu->addAction(scenesMainDock->toggleViewAction());

    //--------------------------------------------

    QDockWidget* scenesSecondDock = new QDockWidget(tr("Сцены под указателем"));
    scenesSecondDock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    addDockWidget(Qt::LeftDockWidgetArea, scenesSecondDock);
    tabifyDockWidget(scenesSecondDock, scenesMainDock);

    _scenesSecondView = new QTableView(this);
    scenesSecondDock->setWidget(_scenesSecondView);

    connect(_scenesSecondView, &QTableView::clicked, this, [this](const QModelIndex& index)
    {
        ScenePtr scene = index.data(Qt::UserRole).value<ScenePtr>();
        emit sceneSelected(scene);
    });
    connect(_scenesSecondView, &QTableView::doubleClicked, this, [this](const QModelIndex& index)
    {
        ScenePtr scene = index.data(Qt::UserRole).value<ScenePtr>();
        _dataManager->zoomToScene(scene);
    });

    _ui.toolsMenu->addAction(scenesSecondDock->toggleViewAction());

    //--------------------------------------------

    _progressBar = new QProgressBar(this);
    _progressBar->setMaximumWidth(200);
    _progressBar->setMinimum(0);
    _progressBar->setMaximum(100);
    _progressBar->setTextVisible(false);
    statusBar()->addWidget(_progressBar);

    //--------------------------------------------

    _mousePosLabel = new QLabel(tr("Указатель:"));
    statusBar()->addWidget(_mousePosLabel, 0);

    //--------------------------------------------

    _mouseReportHandler = new ReportMoveMouseHandler(
        _dataManager->mapNode(),
        [this](const osgEarth::GeoPoint& point)
    {
        if (_ui.selectPointButton->isChecked())
        {
            _ui.longitudeSpinBox->setValue(point.x());
            _ui.latitudeSpinBox->setValue(point.y());
        }

        _mousePosLabel->setText(tr("Указатель: B: %1° L: %2°")
                                .arg(point.y(), 0, 'f', 8)
                                .arg(point.x(), 0, 'f', 8));
    });

    _dataManager->addReportHandler(_mouseReportHandler);

    //--------------------------------------------

    _sceneSelectHandler = new ReportClickMouseHandler(
        _dataManager->mapNode(),
        [this](const osgEarth::GeoPoint& point)
    {
        if (_dataset && _dataset->isInitialized())
        {
            _dataset->selectScenesUnderPointer(point);

            ProxyModel* proxyModel = new ProxyModel(_dataset, this);
            proxyModel->setSourceModel(_scenesMainView->model());

            _scenesSecondView->setModel(proxyModel);
            _scenesSecondView->resizeColumnsToContents();

            connect(_scenesSecondView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onSecondTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));

            if (proxyModel->rowCount() > 0)
            {
                _scenesSecondView->selectionModel()->select(proxyModel->index(0, 0), QItemSelectionModel::ClearAndSelect);

                ScenePtr scene = proxyModel->data(proxyModel->index(0, 0), Qt::UserRole).value<ScenePtr>();
                emit sceneSelected(scene);
            }
        }
    },
        [this]()
    {
    });

    _dataManager->setDefaultActionHandler(_sceneSelectHandler);

    //--------------------------------------------

    _centerSelectHandler = new ReportClickMouseHandler(
        _dataManager->mapNode(),
        [this](const osgEarth::GeoPoint& point)
    {
        _dataManager->setCircleNode(point, _ui.distanceSpinBox->value() * 1000.0);

        QMetaObject::invokeMethod(_ui.selectPointButton, "setChecked", Qt::QueuedConnection, Q_ARG(bool, false));
    },
        [this]()
    {
        QMetaObject::invokeMethod(_ui.selectPointButton, "setChecked", Qt::QueuedConnection, Q_ARG(bool, false));
    });

    connect(_ui.selectPointButton, &QPushButton::toggled, this, [this](bool b)
    {
        if (b)
        {
            _dataManager->setActionHandler(_centerSelectHandler);
        }
        else
        {
            _dataManager->setActionHandler(0);
        }
    });

    //--------------------------------------------

    MetadataWidget* metadataWidget = new MetadataWidget(this);
    connect(this, &MainWindow::sceneSelected, metadataWidget, &MetadataWidget::setScene);

    {
        QDockWidget* dock = new QDockWidget(tr("Метаданные сцены"));
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        dock->setWidget(metadataWidget);
        addDockWidget(Qt::RightDockWidgetArea, dock);

        _ui.toolsMenu->addAction(dock->toggleViewAction());
    }

    //--------------------------------------------

    SceneOperationsWidget* sceneOperationsWidget = new SceneOperationsWidget(_dataManager, this);
    connect(this, &MainWindow::sceneSelected, sceneOperationsWidget, &SceneOperationsWidget::setScene);

    connect(sceneOperationsWidget, &SceneOperationsWidget::progressChanged, _progressBar, &QProgressBar::setValue);
    connect(sceneOperationsWidget, &SceneOperationsWidget::progressReset, _progressBar, &QProgressBar::reset);

    {
        QDockWidget* dock = new QDockWidget(tr("Параметры сохранения"));
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        dock->setWidget(sceneOperationsWidget);
        addDockWidget(Qt::RightDockWidgetArea, dock);

        _ui.toolsMenu->addAction(dock->toggleViewAction());
    }

    //--------------------------------------------


    ProcessingWidget* processingWidget = new ProcessingWidget(_dataManager, this);
    connect(this, &MainWindow::sceneSelected, processingWidget, &ProcessingWidget::setScene);
    connect(sceneOperationsWidget, &SceneOperationsWidget::sceneClipPrepared, processingWidget, &ProcessingWidget::setSceneAndClip);

    {
        QDockWidget* dock = new QDockWidget(tr("Обработка сцены"));
        dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        dock->setWidget(processingWidget);
        addDockWidget(Qt::RightDockWidgetArea, dock);

        _ui.toolsMenu->addAction(dock->toggleViewAction());
    }

    //--------------------------------------------

    connect(this, &MainWindow::sceneSelected, this, [this](const ScenePtr& scene)
    {
        DownloadOverviewOperation* op = new DownloadOverviewOperation(scene, &_dataManager->networkAccessManager(), this);

        connect(op, &DownloadOverviewOperation::finished, this, [op, this](const ScenePtr& scene, const QString& filepath)
        {
            op->deleteLater();
            op->setParent(0);

            _dataManager->showOverview(scene, filepath);
        });

        connect(op, &DownloadOverviewOperation::error, this, [op, this](const QString& text)
        {
            op->deleteLater();
            op->setParent(0);

            qDebug() << text;
        });

        op->start();
    });
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    QSettings settings;
    settings.setValue("MainWindow/geometry", saveGeometry());
    //settings.setValue("MainWindow/windowState", saveState());

    QMainWindow::closeEvent(event);
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

    settings.setValue("Query/hyperionChecked", _ui.hyperionCheckBox->isChecked());
    settings.setValue("Query/avirisChecked", _ui.avirisCheckBox->isChecked());

    settings.setValue("Query/sceneIdEnabled", _ui.sceneIdCheckBox->isChecked());
    settings.setValue("Query/sceneIdText", _ui.sceneIdLineEdit->text());

    settings.setValue("Query/hasSceneEnabled", _ui.hasSceneCheckBox->isChecked());

    settings.setValue("Query/dateEnabled", _ui.dateGroupBox->isChecked());
    settings.setValue("Query/dateFrom", _ui.dateTimeEditFrom->dateTime());
    settings.setValue("Query/dateTo", _ui.dateTimeEditTo->dateTime());

    settings.setValue("Query/sunAzimuthEnabled", _ui.sunAzimuthGroupBox->isChecked());
    settings.setValue("Query/sunAzimuthFrom", _ui.sunAzimuthFromSpinBox->value());
    settings.setValue("Query/sunAzimuthTo", _ui.sunAzimuthToSpinBox->value());

    settings.setValue("Query/sunElevationEnabled", _ui.sunElevationGroupBox->isChecked());
    settings.setValue("Query/sunElevationFrom", _ui.sunElevationFromSpinBox->value());
    settings.setValue("Query/sunElevationTo", _ui.sunElevationToSpinBox->value());

    settings.setValue("Query/pixelSizeEnabled", _ui.pixelSizeGroupBox->isChecked());
    settings.setValue("Query/pixelSizeFrom", _ui.pixelSizeFromSpinBox->value());
    settings.setValue("Query/pixelSizeTo", _ui.pixelSizeToSpinBox->value());

    settings.setValue("Query/distanceEnabled", _ui.distanceGroupBox->isChecked());
    settings.setValue("Query/centerLongitude", _ui.longitudeSpinBox->value());
    settings.setValue("Query/centerLatitude", _ui.latitudeSpinBox->value());
    settings.setValue("Query/distanceValue", _ui.distanceSpinBox->value());

    //-- Hyperion

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

    //-- AVIRIS

    settings.setValue("Query/siteNameEnabled", _ui.siteNameCheckBox->isChecked());
    settings.setValue("Query/siteNameText", _ui.siteNameLineEdit->text());

    settings.setValue("Query/rotationEnabled", _ui.rotationGroupBox->isChecked());
    settings.setValue("Query/rotationFrom", _ui.rotationFromSpinBox->value());
    settings.setValue("Query/rotationTo", _ui.rotationToSpinBox->value());

    settings.setValue("Query/meanElevationEnabled", _ui.meanElevationGroupBox->isChecked());
    settings.setValue("Query/meanElevationFrom", _ui.meanElevationFromSpinBox->value());
    settings.setValue("Query/meanElevationTo", _ui.meanElevationToSpinBox->value());

    //-----------------------------------------------------

    if (_ui.distanceGroupBox->isChecked())
    {
        _dataManager->setCircleNode(GeoPoint(_dataManager->mapNode()->getMapSRS(), _ui.longitudeSpinBox->value(), _ui.latitudeSpinBox->value(), 0.0, osgEarth::ALTMODE_ABSOLUTE), _ui.distanceSpinBox->value() * 1000);
    }
    else
    {
        _dataManager->removeCircleNode();
    }

    //-----------------------------------------------------    

    _dataset = std::make_shared<DataSet>();

    SensorQueryPtr hyperionQuery, avirisQuery;
    std::vector<SensorQueryPtr> activeQueries;

    if (_ui.hyperionCheckBox->isChecked())
    {
        hyperionQuery = std::make_shared<HyperionQuery>();
        activeQueries.push_back(hyperionQuery);

        _dataset->addSensor(hyperionQuery);
    }
    if (_ui.avirisCheckBox->isChecked())
    {
        avirisQuery = std::make_shared<AvirisQuery>();
        activeQueries.push_back(avirisQuery);

        _dataset->addSensor(avirisQuery);
    }

    //-- Common

    if (_ui.sceneIdCheckBox->isChecked())
    {
        for (const auto& q : activeQueries)
        {
            q->addCondition(QString("sceneid='%0'").arg(_ui.sceneIdLineEdit->text()));
        }
    }

    if (_ui.hasSceneCheckBox->isChecked())
    {
        for (const auto& q : activeQueries)
        {
            q->addCondition(QString("hasscene"));
        }
    }

    if (_ui.dateGroupBox->isChecked())
    {
        for (const auto& q : activeQueries)
        {
            q->addCondition(QString("scenetime>='%0'::timestamp without time zone and scenetime<='%1'::timestamp without time zone").arg(_ui.dateTimeEditFrom->dateTime().toString(Qt::ISODate)).arg(_ui.dateTimeEditTo->dateTime().toString(Qt::ISODate)));
        }
    }

    if (_ui.sunAzimuthGroupBox->isChecked())
    {
        for (const auto& q : activeQueries)
        {
            q->addCondition(QString("sunazimuth>=%0 and sunazimuth<=%1").arg(_ui.sunAzimuthFromSpinBox->value(), 0, 'f', 7).arg(_ui.sunAzimuthToSpinBox->value(), 0, 'f', 7));
        }
    }

    if (_ui.sunElevationGroupBox->isChecked())
    {
        for (const auto& q : activeQueries)
        {
            q->addCondition(QString("sunelevation>=%0 and sunelevation<=%1").arg(_ui.sunElevationFromSpinBox->value(), 0, 'f', 7).arg(_ui.sunElevationToSpinBox->value(), 0, 'f', 7));
        }
    }

    if (_ui.pixelSizeGroupBox->isChecked())
    {
        for (const auto& q : activeQueries)
        {
            q->addCondition(QString("pixelsize>=%0 and pixelsize<=%1").arg(_ui.pixelSizeFromSpinBox->value(), 0, 'f', 7).arg(_ui.pixelSizeToSpinBox->value(), 0, 'f', 7));
        }
    }

    if (_ui.distanceGroupBox->isChecked())
    {
        for (const auto& q : activeQueries)
        {
            q->addCondition(QString("ST_DWithin(bounds,ST_GeographyFromText('SRID=4326;POINT(%0 %1)'),%3)").arg(_ui.longitudeSpinBox->value(), 0, 'f', 12).arg(_ui.latitudeSpinBox->value(), 0, 'f', 12).arg(_ui.distanceSpinBox->value() * 1000));
        }
    }

    //-- Hyperion

    if (hyperionQuery && activeQueries.size() == 1)
    {
        if (_ui.inclinationGroupBox->isChecked())
        {
            hyperionQuery->addCondition(QString("satelliteinclination>=%0 and satelliteinclination<=%1").arg(_ui.inclinationFromSpinBox->value(), 0, 'f', 7).arg(_ui.inclinationToSpinBox->value(), 0, 'f', 7));
        }

        if (_ui.lookAngleGroupBox->isChecked())
        {
            hyperionQuery->addCondition(QString("lookangle>=%0 and lookangle<=%1").arg(_ui.lookAngleFromSpinBox->value(), 0, 'f', 7).arg(_ui.lookAngleToSpinBox->value(), 0, 'f', 7));
        }

        if (_ui.processingLevelGroupBox->isChecked())
        {
            if (_ui.l1RRadioButton->isChecked())
            {
                hyperionQuery->addCondition("(processinglevel='L1R Product Available' OR processinglevel='L1Gst Product Available' OR processinglevel='L1T Product Available')");
            }
            else if (_ui.l1GstRadioButton->isChecked())
            {
                hyperionQuery->addCondition("(processinglevel='L1Gst Product Available' OR processinglevel='L1T Product Available')");
            }
            else if (_ui.l1TRadioButton->isChecked())
            {
                hyperionQuery->addCondition("processinglevel='L1T Product Available'");
            }
            else
            {
                qDebug() << "Wrong processing level";
            }
        }

        if (_ui.cloudnessCheckBox->isChecked())
        {
            hyperionQuery->addCondition(QString("cloudmax<=%0").arg(_ui.cloudnessComboBox->currentText().toInt()));
        }

        if (_ui.orbitPathCheckBox->isChecked())
        {
            hyperionQuery->addCondition(QString("orbitpath=%0").arg(_ui.orbitPathSpinBox->value()));
        }

        if (_ui.orbitRowCheckBox->isChecked())
        {
            hyperionQuery->addCondition(QString("orbitrow=%0").arg(_ui.orbitRowSpinBox->value()));
        }

        if (_ui.targetPathCheckBox->isChecked())
        {
            hyperionQuery->addCondition(QString("targetpath=%0").arg(_ui.targetPathSpinBox->value()));
        }

        if (_ui.targetRowCheckBox->isChecked())
        {
            hyperionQuery->addCondition(QString("targetrow=%0").arg(_ui.targetRowSpinBox->value()));
        }
    }

    //-- AVIRIS

    if (avirisQuery && activeQueries.size() == 1)
    {
        if (_ui.siteNameCheckBox->isChecked())
        {
            for (const auto& q : activeQueries)
            {
                q->addCondition(QString("upper(sitename) LIKE upper('%%0%')").arg(_ui.siteNameLineEdit->text()));
            }
        }

        if (_ui.rotationGroupBox->isChecked())
        {
            for (const auto& q : activeQueries)
            {
                q->addCondition(QString("scenerotation>=%0 and scenerotation<=%1").arg(_ui.rotationFromSpinBox->value(), 0, 'f', 7).arg(_ui.rotationToSpinBox->value(), 0, 'f', 7));
            }
        }

        if (_ui.meanElevationGroupBox->isChecked())
        {
            for (const auto& q : activeQueries)
            {
                q->addCondition(QString("meansceneelev>=%0 and meansceneelev<=%1").arg(_ui.meanElevationFromSpinBox->value(), 0, 'f', 7).arg(_ui.meanElevationToSpinBox->value(), 0, 'f', 7));
            }
        }
    }

    //--

    QtConcurrent::run([this]()
    {
        _dataset->selectScenes();

        _dataManager->setDataSet(_dataset);

        QMetaObject::invokeMethod(this, "finishLoadScenes", Qt::QueuedConnection);
    });
}

void MainWindow::finishLoadScenes()
{
    TableModel* tableModel = new TableModel(_dataset, this);
    _scenesMainView->setModel(tableModel);
    _scenesMainView->resizeColumnsToContents();

    connect(_scenesMainView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection&, const QItemSelection&)), this, SLOT(onMainTableViewSelectionChanged(const QItemSelection&, const QItemSelection&)));

    _progressBar->setMaximum(100);
    _ui.dockWidget->setEnabled(true);
}

//-------------------------------------------------------

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
    if (!proxyModel)
    {
        return;
    }

    QItemSelection sourceSelection = proxyModel->mapSelectionToSource(selected);

    _scenesMainView->selectionModel()->select(sourceSelection, QItemSelectionModel::ClearAndSelect);
    if (!sourceSelection.empty())
    {
        _scenesMainView->scrollTo(sourceSelection.first().indexes()[0]);
    }
}
