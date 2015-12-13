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
_sitenameProp(0),
_commentsProp(0),
_investigatorProp(0),
_scenerotationProp(0),
_tapeProp(0),
_geoverProp(0),
_rdnverProp(0),
_meanSceneElevProp(0),
_minSceneElevProp(0),
_maxSceneElevProp(0),
_flightProp(0),
_runProp(0),
//_overviewDownloadLabel(0),
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

    _pixelSizeProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Размер пикселя"));
    _pixelSizeProp->setAttribute("readOnly", true);
    _browser->addProperty(_pixelSizeProp);

    _sunAzimuthProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Азимут Солнца"));
    _sunAzimuthProp->setAttribute("readOnly", true);
    _browser->addProperty(_sunAzimuthProp);

    _sunElevationProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Высота Солнца"));
    _sunElevationProp->setAttribute("readOnly", true);
    _browser->addProperty(_sunElevationProp);

    //-- Hyperion
        
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

    _cloudnessProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Процент облачности"));
    _cloudnessProp->setAttribute("readOnly", true);
    _browser->addProperty(_cloudnessProp);   

    _inclinationProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Наклонение орбиты спутника"));
    _inclinationProp->setAttribute("readOnly", true);
    _browser->addProperty(_inclinationProp);

    _lookAngleProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Угол между надиром и центром сцены"));
    _lookAngleProp->setAttribute("readOnly", true);
    _browser->addProperty(_lookAngleProp);

    //-- AVIRIS

    _sitenameProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Имя объекта"));
    _sitenameProp->setAttribute("readOnly", true);
    _browser->addProperty(_sitenameProp);

    _commentsProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Комментарии"));
    _commentsProp->setAttribute("readOnly", true);
    _browser->addProperty(_commentsProp);

    _investigatorProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Исследователь"));
    _investigatorProp->setAttribute("readOnly", true);
    _browser->addProperty(_investigatorProp);

    _scenerotationProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Вращение сцены"));
    _scenerotationProp->setAttribute("readOnly", true);
    _browser->addProperty(_scenerotationProp);

    _tapeProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Номер ленты"));
    _tapeProp->setAttribute("readOnly", true);
    _browser->addProperty(_tapeProp);

    _geoverProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Геокоррекция"));
    _geoverProp->setAttribute("readOnly", true);
    _browser->addProperty(_geoverProp);

    _rdnverProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("RDN"));
    _rdnverProp->setAttribute("readOnly", true);
    _browser->addProperty(_rdnverProp);

    _meanSceneElevProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Средняя высота сцены"));
    _meanSceneElevProp->setAttribute("readOnly", true);
    _browser->addProperty(_meanSceneElevProp);

    _minSceneElevProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Минимальная высота сцены"));
    _minSceneElevProp->setAttribute("readOnly", true);
    _browser->addProperty(_minSceneElevProp);

    _maxSceneElevProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Максимальная высота сцены"));
    _maxSceneElevProp->setAttribute("readOnly", true);
    _browser->addProperty(_maxSceneElevProp);

    _flightProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Номер вылета"));
    _flightProp->setAttribute("readOnly", true);
    _browser->addProperty(_flightProp);

    _runProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Номер захода"));
    _runProp->setAttribute("readOnly", true);
    _browser->addProperty(_runProp);

    //--
            
    //_overviewDownloadLabel = new QLabel(QString::fromUtf8("Скачать обзор (<a href='http://google.ru'>ссылка</a>)"));
    //_overviewDownloadLabel->setOpenExternalLinks(true);
    //layout->addWidget(_overviewDownloadLabel);

    _sceneDownloadLabel = new QLabel(QString::fromUtf8("Ссылка для скачивания отсутствует"));
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

    _sceneidProp->setValue(scene->sceneId);
    _datetimeProp->setValue(scene->sceneTime.toString(Qt::ISODate));

    _pixelSizeProp->setValue(scene->pixelSize);

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

    //-- Hyperion

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

    //-- AVIRIS

    if (scene->sitename)
    {
        _sitenameProp->setValue(scene->sitename.get());
        _sitenameProp->setEnabled(true);
    }
    else
    {
        _sitenameProp->setValue(QVariant());
        _sitenameProp->setEnabled(false);
    }

    if (scene->comments)
    {
        _commentsProp->setValue(scene->comments.get());
        _commentsProp->setEnabled(true);
    }
    else
    {
        _commentsProp->setValue(QVariant());
        _commentsProp->setEnabled(false);
    }

    if (scene->investigator)
    {
        _investigatorProp->setValue(scene->investigator.get());
        _investigatorProp->setEnabled(true);
    }
    else
    {
        _investigatorProp->setValue(QVariant());
        _investigatorProp->setEnabled(false);
    }

    if (scene->scenerotation)
    {
        _scenerotationProp->setValue(scene->scenerotation.get());
        _scenerotationProp->setEnabled(true);
    }
    else
    {
        _scenerotationProp->setValue(QVariant());
        _scenerotationProp->setEnabled(false);
    }

    if (scene->tape)
    {
        _tapeProp->setValue(scene->tape.get());
        _tapeProp->setEnabled(true);
    }
    else
    {
        _tapeProp->setValue(QVariant());
        _tapeProp->setEnabled(false);
    }

    if (scene->geover)
    {
        _geoverProp->setValue(scene->geover.get());
        _geoverProp->setEnabled(true);
    }
    else
    {
        _geoverProp->setValue(QVariant());
        _geoverProp->setEnabled(false);
    }

    if (scene->rdnver)
    {
        _rdnverProp->setValue(scene->rdnver.get());
        _rdnverProp->setEnabled(true);
    }
    else
    {
        _rdnverProp->setValue(QVariant());
        _rdnverProp->setEnabled(false);
    }

    if (scene->meansceneelev)
    {
        _meanSceneElevProp->setValue(scene->meansceneelev.get());
        _meanSceneElevProp->setEnabled(true);
    }
    else
    {
        _meanSceneElevProp->setValue(QVariant());
        _meanSceneElevProp->setEnabled(false);
    }

    if (scene->minsceneelev)
    {
        _minSceneElevProp->setValue(scene->minsceneelev.get());
        _minSceneElevProp->setEnabled(true);
    }
    else
    {
        _minSceneElevProp->setValue(QVariant());
        _minSceneElevProp->setEnabled(false);
    }

    if (scene->maxsceneelev)
    {
        _maxSceneElevProp->setValue(scene->maxsceneelev.get());
        _maxSceneElevProp->setEnabled(true);
    }
    else
    {
        _maxSceneElevProp->setValue(QVariant());
        _maxSceneElevProp->setEnabled(false);
    }

    if (scene->flight)
    {
        _flightProp->setValue(scene->flight.get());
        _flightProp->setEnabled(true);
    }
    else
    {
        _flightProp->setValue(QVariant());
        _flightProp->setEnabled(false);
    }

    if (scene->run)
    {
        _runProp->setValue(scene->run.get());
        _runProp->setEnabled(true);
    }
    else
    {
        _runProp->setValue(QVariant());
        _runProp->setEnabled(false);
    }

    //--
            
    if (scene->hasScene)
    {
        //_overviewDownloadLabel->setText(QString::fromUtf8("Скачать обзор с сервера USGS (<a href='http://earthexplorer.usgs.gov/metadata/1854/%0/'>ссылка</a>)").arg(scene->sceneId));
        //_overviewDownloadLabel->setVisible(true);

        _sceneDownloadLabel->setVisible(false);
    }
    else
    {
        //_overviewDownloadLabel->setVisible(true);        

        if (scene->sceneUrl)
        {
            _sceneDownloadLabel->setText(QString::fromUtf8("Сцена отсутствует на геопортале. <a href='%0'>Скачать</a> сцену вручную").arg(*scene->sceneUrl));
        }
        else
        {
            _sceneDownloadLabel->setText(QString::fromUtf8("Сцена отсутствует на геопортале."));
        }
        _sceneDownloadLabel->setVisible(true);
    }
}
