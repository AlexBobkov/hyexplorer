#include "SceneOperationsWidget.hpp"
#include "Storage.hpp"

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

namespace
{
    void openExplorer(const QString& path)
    {
        if (QFileInfo(path).isDir())
        {
            QDesktopServices::openUrl(QUrl(QString("file:///") + path.toLocal8Bit()));
        }
        else
        {            
            QProcess::startDetached(QString("explorer.exe /select,\"%0\"").arg(QDir::toNativeSeparators(path.toLocal8Bit())));
        }
    }
}

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
    connect(_ui.downloadButton, &QPushButton::clicked, this, &SceneOperationsWidget::download);    
    connect(_ui.openFolderButton, &QPushButton::clicked, this, &SceneOperationsWidget::openFolder);    

    connect(_ui.importButton, &QPushButton::clicked, this, [this]()
    {
        _ui.importButton->setVisible(false);

        emit importSceneRequested(_scene);
    });

    //---------------------------------------

    ClipInfoPtr clipInfo = _dataManager->clipInfo();
    if (clipInfo)
    {
        _ui.leftSpinBox->setValue(clipInfo->bounds().xMin());
        _ui.rightSpinBox->setValue(clipInfo->bounds().xMax());
        _ui.topSpinBox->setValue(clipInfo->bounds().yMax());
        _ui.bottomSpinBox->setValue(clipInfo->bounds().yMin());

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

        ClipInfoPtr clipInfo = std::make_shared<ClipInfo>(b);
        clipInfo->setMinBand(_ui.fromSpinBox->value());
        clipInfo->setMaxBand(_ui.toSpinBox->value());

        _dataManager->setClipInfo(clipInfo);

        emit rectangleChanged(b);
    };

    connect(_ui.leftSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.rightSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.topSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.bottomSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
}

void SceneOperationsWidget::download()
{
    if (_ui.fullSizeRadioButton->isChecked())
    {
        _ui.downloadButton->setEnabled(false);

        emit downloadSceneRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value(), ClipInfoPtr());
    }
    else
    {
        if (_dataManager->clipInfo())
        {
            if (_dataManager->clipInfo()->bounds().intersects(_scene->geometry()->getBounds()))
            {
                _ui.downloadButton->setEnabled(false);

                emit downloadSceneRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value(), _dataManager->clipInfo());
            }
            else
            {
                QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Фрагмент не пересекает границы сцены"));
            }
        }
        else
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Сначала выделите фрагмент"));
        }
    }
}

void SceneOperationsWidget::onSceneDownloaded(const ScenePtr& scene, bool result, const QString& message)
{
    _ui.downloadButton->setEnabled(true);

    if (result)
    {
        _ui.openFolderButton->setEnabled(true);

        QMessageBox::information(qApp->activeWindow(), tr("Выбранные каналы получены"), message);

        openFolder();
    }
    else
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Ошибка получения сцены"), message);
    }
}

void SceneOperationsWidget::setScene(const ScenePtr& scene)
{
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
    _ui.openFolderButton->setEnabled(false);

    _ui.leftSpinBox->setMaximum(b.xMax());
    _ui.rightSpinBox->setMinimum(b.xMin());
    _ui.topSpinBox->setMinimum(b.yMin());
    _ui.bottomSpinBox->setMaximum(b.yMax());

    _ui.leftSpinBox->setValue(b.xMin());
    _ui.rightSpinBox->setValue(b.xMax());
    _ui.topSpinBox->setValue(b.yMax());
    _ui.bottomSpinBox->setValue(b.yMin());

    ClipInfoPtr clipInfo = std::make_shared<ClipInfo>(b);
    clipInfo->setMinBand(_ui.fromSpinBox->value());
    clipInfo->setMaxBand(_ui.toSpinBox->value());

    _dataManager->setClipInfo(clipInfo);
}

void SceneOperationsWidget::onRectangleSelectFailed()
{
    _ui.selectFragmentButton->setChecked(false);
}

void SceneOperationsWidget::openFolder()
{
    if (_ui.fullSizeRadioButton->isChecked())
    {
        openExplorer(Storage::sceneBandDir(_scene).path());
    }
    else
    {
        if (_dataManager->clipInfo())
        {
            openExplorer(Storage::sceneBandDir(_scene, _dataManager->clipInfo()).path());
        }
    }
}
