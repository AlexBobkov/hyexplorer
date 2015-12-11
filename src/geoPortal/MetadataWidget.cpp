#include "MetadataWidget.hpp"

#include <QStandardPaths>
#include <QVBoxLayout>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QMessageBox>
#include <QTextStream>

using namespace portal;

MetadataWidget::MetadataWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(),
_dataManager(dataManager),
_browser(0),
_variantManager(0),
_sceneidProp(0),
_datetimeProp(0),
_cloudnessProp(0),
_orbitPathProp(0),
_orbitRowProp(0),
_targetPathProp(0),
_targetRowProp(0),
_processingLevelProp(0),
_sunAzimuthProp(0),
_sunElevationProp(0),
_inclinationProp(0),
_lookAngleProp(0),
_overviewDownloadLabel(0),
_sceneDownloadLabel(0)
{
    initUi();
}

MetadataWidget::~MetadataWidget()
{
}

void MetadataWidget::initUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    _variantManager = new QtVariantPropertyManager(this);

    _browser = new QtTreePropertyBrowser(this);
    _browser->setFactoryForManager(_variantManager, new QtVariantEditorFactory(this));
    layout->addWidget(_browser);

    _sceneidProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Идентификатор"));
    _sceneidProp->setAttribute("readOnly", true);
    _browser->addProperty(_sceneidProp);

    //_datetimeProp = _variantManager->addProperty(QVariant::DateTime, QString::fromUtf8("Scene time"));
    _datetimeProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Дата и время съемки"));
    _datetimeProp->setAttribute("readOnly", true);
    _browser->addProperty(_datetimeProp);

    _cloudnessProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Процент облачности"));
    _cloudnessProp->setAttribute("readOnly", true);
    _browser->addProperty(_cloudnessProp);

    _orbitPathProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Номер витка"));
    _orbitPathProp->setAttribute("readOnly", true);
    _browser->addProperty(_orbitPathProp);

    _orbitRowProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Номер ряда"));
    _orbitRowProp->setAttribute("readOnly", true);
    _browser->addProperty(_orbitRowProp);

    _targetPathProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Номер целевого витка"));
    _targetPathProp->setAttribute("readOnly", true);
    _browser->addProperty(_targetPathProp);

    _targetRowProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Номер целевого ряда"));
    _targetRowProp->setAttribute("readOnly", true);
    _browser->addProperty(_targetRowProp);

    _processingLevelProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Уровень обработки"));
    _processingLevelProp->setAttribute("readOnly", true);
    _browser->addProperty(_processingLevelProp);

    _sunAzimuthProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Азимут Солнца"));
    _sunAzimuthProp->setAttribute("readOnly", true);
    _browser->addProperty(_sunAzimuthProp);

    _sunElevationProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Высота Солнца"));
    _sunElevationProp->setAttribute("readOnly", true);
    _browser->addProperty(_sunElevationProp);

    _inclinationProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Наклонение орбиты спутника"));
    _inclinationProp->setAttribute("readOnly", true);
    _browser->addProperty(_inclinationProp);

    _lookAngleProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Угол между надиром и центром сцены"));
    _lookAngleProp->setAttribute("readOnly", true);
    _browser->addProperty(_lookAngleProp);
       
        
    _overviewDownloadLabel = new QLabel(QString::fromUtf8("Скачать обзор (<a href='http://google.ru'>ссылка</a>)"));
    _overviewDownloadLabel->setOpenExternalLinks(true);
    layout->addWidget(_overviewDownloadLabel);

    _sceneDownloadLabel = new QLabel(QString::fromUtf8("Скачать сцену (<a href='http://google.ru'>ссылка</a>)"));
    _sceneDownloadLabel->setOpenExternalLinks(true);
    layout->addWidget(_sceneDownloadLabel);
}

void MetadataWidget::setScene(const ScenePtr& scene)
{
    if (_scene == scene)
    {
        return;
    }

    _scene = scene;

    _sceneidProp->setValue(scene->sceneid);
    _datetimeProp->setValue(scene->sceneTime.toString(Qt::ISODate));

    if (scene->cloundMin && scene->cloundMax)
    {
        _cloudnessProp->setValue(QString("%0% to %1%").arg(scene->cloundMin.get()).arg(scene->cloundMax.get()));
        _cloudnessProp->setEnabled(true);
    }
    else
    {
        _cloudnessProp->setValue(QString());
        _cloudnessProp->setEnabled(false);
    }

    if (scene->orbitPath)
    {
        _orbitPathProp->setValue(scene->orbitPath.get());
        _orbitPathProp->setEnabled(true);
    }
    else
    {
        _orbitPathProp->setValue(0);
        _orbitPathProp->setEnabled(false);
    }

    if (scene->orbitRow)
    {
        _orbitRowProp->setValue(scene->orbitRow.get());
        _orbitRowProp->setEnabled(true);
    }
    else
    {
        _orbitRowProp->setValue(0);
        _orbitRowProp->setEnabled(false);
    }

    if (scene->targetPath)
    {
        _targetPathProp->setValue(scene->targetPath.get());
        _targetPathProp->setEnabled(true);
    }
    else
    {
        _targetPathProp->setValue(0);
        _targetPathProp->setEnabled(false);
    }

    if (scene->targetRow)
    {
        _targetRowProp->setValue(scene->targetRow.get());
        _targetRowProp->setEnabled(true);
    }
    else
    {
        _targetRowProp->setValue(0);
        _targetRowProp->setEnabled(false);
    }

    if (scene->processingLevel)
    {
        _processingLevelProp->setValue(scene->processingLevel.get());
        _processingLevelProp->setEnabled(true);
    }
    else
    {
        _processingLevelProp->setValue(QString());
        _processingLevelProp->setEnabled(false);
    }

    if (scene->sunAzimuth)
    {
        _sunAzimuthProp->setValue(scene->sunAzimuth.get());
        _sunAzimuthProp->setEnabled(true);
    }
    else
    {
        _sunAzimuthProp->setValue(0.0);
        _sunAzimuthProp->setEnabled(false);
    }

    if (scene->sunElevation)
    {
        _sunElevationProp->setValue(scene->sunElevation.get());
        _sunElevationProp->setEnabled(true);
    }
    else
    {
        _sunElevationProp->setValue(0.0);
        _sunElevationProp->setEnabled(false);
    }

    if (scene->inclination)
    {
        _inclinationProp->setValue(scene->inclination.get());
        _inclinationProp->setEnabled(true);
    }
    else
    {
        _inclinationProp->setValue(0.0);
        _inclinationProp->setEnabled(false);
    }

    if (scene->lookAngle)
    {
        _lookAngleProp->setValue(scene->lookAngle.get());
        _lookAngleProp->setEnabled(true);
    }
    else
    {
        _lookAngleProp->setValue(0.0);
        _lookAngleProp->setEnabled(false);
    }

    _overviewDownloadLabel->setText(QString::fromUtf8("Скачать обзор с сервера USGS (<a href='http://earthexplorer.usgs.gov/metadata/1854/%0/'>ссылка</a>)").arg(scene->sceneid));
    _sceneDownloadLabel->setText(QString::fromUtf8("Скачать сцену с сервера USGS (<a href='http://earthexplorer.usgs.gov/download/options/1854/%0/'>ссылка</a>)").arg(scene->sceneid));

    if (scene->hasScene)
    {
        _overviewDownloadLabel->setVisible(true);
        _sceneDownloadLabel->setVisible(false);
    }
    else
    {
        _overviewDownloadLabel->setVisible(true);
        _sceneDownloadLabel->setVisible(true);
    }
}
