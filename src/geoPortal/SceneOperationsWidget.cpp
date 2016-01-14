#include "SceneOperationsWidget.hpp"
#include "Storage.hpp"
#include "Utils.hpp"
#include "Operations.hpp"

#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QSqlQueryModel>
#include <QTableView>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>

using namespace portal;

SceneOperationsWidget::SceneOperationsWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager)
{
    initUi();
}

SceneOperationsWidget::~SceneOperationsWidget()
{
}

void SceneOperationsWidget::initUi()
{
    _ui.setupUi(this);

    _ui.importButton->setVisible(false);
    _ui.bandsGroupBox->setEnabled(false);
    _ui.boundsGroupBox->setEnabled(false);
    _ui.downloadButton->setEnabled(false);
    _ui.openFolderButton->setEnabled(false);

    //---------------------------------------

    connect(_ui.importButton, &QPushButton::clicked, this, [this]()
    {
        _ui.importButton->setVisible(false);

        emit importSceneRequested(_scene);
    });

    //---------------------------------------

    QSettings settings;
    _ui.fromSpinBox->setValue(settings.value("SceneOperationsWidget/fromValue", 1).toInt());
    _ui.toSpinBox->setValue(settings.value("SceneOperationsWidget/toValue", 1).toInt());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());
       
    connect(_ui.fromSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("SceneOperationsWidget/fromValue", _ui.fromSpinBox->value());

        _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());
    });

    connect(_ui.toSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("SceneOperationsWidget/toValue", _ui.toSpinBox->value());

        _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    });

    //---------------------------------------

    connect(_ui.selectFragmentButton, &QPushButton::clicked, this, &SceneOperationsWidget::selectRectangleRequested);

    //---------------------------------------

    connect(_ui.downloadButton, &QPushButton::clicked, this, [this]()
    {
        if (_ui.fragmentRadioButton->isChecked() && !_dataManager->bounds())
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Сначала выделите фрагмент"));
            return;
        }

        if (_ui.fragmentRadioButton->isChecked() && !_dataManager->bounds()->intersects(_scene->geometry()->getBounds()))
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Фрагмент не пересекает границы сцены"));
            return;
        }

        setEnabled(false);

        if (_ui.fragmentRadioButton->isChecked())
        {
            _clipInfo = std::make_shared<ClipInfo>(_scene, *_dataManager->bounds());
        }
        else
        {
            _clipInfo = std::make_shared<ClipInfo>(_scene);
        }
        _clipInfo->setMinBand(_ui.fromSpinBox->value());
        _clipInfo->setMaxBand(_ui.toSpinBox->value());
                
        DownloadSceneOperation* op = new DownloadSceneOperation(_scene, _clipInfo, &_dataManager->networkAccessManager(), this);
        connect(op, &DownloadSceneOperation::finished, this, [op, this](const ScenePtr& scene, const ClipInfoPtr& clipInfo)
        {
            op->deleteLater();
            op->setParent(0);

            QMessageBox::information(qApp->activeWindow(), tr("Скачивание сцены"), tr("Выбранные каналы скачаны"));

            setEnabled(true);
            
            _ui.openFolderButton->setEnabled(true);
            openExplorer(Storage::sceneBandDir(scene, clipInfo).path());

            emit sceneClipPrepared(scene, clipInfo);
        });

        connect(op, &DownloadSceneOperation::error, this, [op, this](const QString& text)
        {
            op->deleteLater();
            op->setParent(0);

            QMessageBox::warning(qApp->activeWindow(), tr("Скачивание сцены"), text);

            setEnabled(true);
        });
    });
    
    connect(_ui.openFolderButton, &QPushButton::clicked, this, [this]()
    {
        openExplorer(Storage::sceneBandDir(_scene, _clipInfo).path());
    });    

    //---------------------------------------

    boost::optional<osgEarth::Bounds> bounds = _dataManager->bounds();
    if (bounds)
    {
        _ui.leftSpinBox->setValue(bounds->xMin());
        _ui.rightSpinBox->setValue(bounds->xMax());
        _ui.topSpinBox->setValue(bounds->yMax());
        _ui.bottomSpinBox->setValue(bounds->yMin());

        _ui.leftSpinBox->setMaximum(_ui.rightSpinBox->value());
        _ui.rightSpinBox->setMinimum(_ui.leftSpinBox->value());
        _ui.topSpinBox->setMinimum(_ui.bottomSpinBox->value());
        _ui.bottomSpinBox->setMaximum(_ui.topSpinBox->value());
    }

    auto clipSpinBoxCB = [this](double d)
    {
        _ui.openFolderButton->setEnabled(false);

        _ui.leftSpinBox->setMaximum(_ui.rightSpinBox->value());
        _ui.rightSpinBox->setMinimum(_ui.leftSpinBox->value());
        _ui.topSpinBox->setMinimum(_ui.bottomSpinBox->value());
        _ui.bottomSpinBox->setMaximum(_ui.topSpinBox->value());

        osgEarth::Bounds b(_ui.leftSpinBox->value(), _ui.bottomSpinBox->value(), _ui.rightSpinBox->value(), _ui.topSpinBox->value());
        
        _dataManager->setBounds(b);

        emit rectangleChanged(b);
    };

    connect(_ui.leftSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.rightSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.topSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.bottomSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
}

void SceneOperationsWidget::setScene(const ScenePtr& scene)
{
    if (!isEnabled())
    {
        qDebug() << "Attemp to change the scene during the downloading";
        return;
    }

    if (_scene == scene)
    {
        return;
    }

    _scene = scene;

    if (scene->hasScene())
    {
        _ui.statusLabel->setText(tr("Сцена находится на нашем сервере и доступна для работы"));

        _ui.importButton->setVisible(false);
        _ui.bandsGroupBox->setEnabled(true);
        _ui.boundsGroupBox->setEnabled(true);
        _ui.downloadButton->setEnabled(true);
    }
    else
    {
        if (!scene->sceneUrl().isNull() && !scene->sceneUrl().isEmpty())
        {
            _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере, но вы можете <a href='%0'>скачать</a> сцену вручную").arg(scene->sceneUrl()));
        }
        else
        {
            _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере и не доступна для работы"));
        }

        if (scene->sensor() == "Hyperion" && scene->attrib("processinglevel") == "L1T Product Available")
        {
            _ui.importButton->setVisible(true);
        }
        else
        {
            _ui.importButton->setVisible(false);
        }

        _ui.bandsGroupBox->setEnabled(false);
        _ui.boundsGroupBox->setEnabled(false);
        _ui.downloadButton->setEnabled(false);
    }

    _ui.openFolderButton->setEnabled(false);
}

void SceneOperationsWidget::onRectangleSelected(const osgEarth::Bounds& b)
{
    _ui.selectFragmentButton->setChecked(false);

    _ui.leftSpinBox->setMaximum(b.xMax());
    _ui.rightSpinBox->setMinimum(b.xMin());
    _ui.topSpinBox->setMinimum(b.yMin());
    _ui.bottomSpinBox->setMaximum(b.yMax());

    _ui.leftSpinBox->setValue(b.xMin());
    _ui.rightSpinBox->setValue(b.xMax());
    _ui.topSpinBox->setValue(b.yMax());
    _ui.bottomSpinBox->setValue(b.yMin());
    
    _dataManager->setBounds(b);
}

void SceneOperationsWidget::onRectangleSelectFailed()
{
    _ui.selectFragmentButton->setChecked(false);
}

