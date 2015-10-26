#include "MetadataWidget.hpp"

#include <osg/Image>
#include <osgEarthAnnotation/ImageOverlay>

#include <QStandardPaths>
#include <QVBoxLayout>
#include <QDateTime>
#include <QDir>

#include <iostream>

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace portal;

MetadataWidget::MetadataWidget() :
QWidget(),
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

    _sceneidProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Scene Id"));
    _sceneidProp->setAttribute("readOnly", true);
    _browser->addProperty(_sceneidProp);

    _datetimeProp = _variantManager->addProperty(QVariant::DateTime, QString::fromUtf8("Scene time"));
    _datetimeProp->setAttribute("readOnly", true);
    _browser->addProperty(_datetimeProp);

    _cloudnessProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Cloud cover"));
    _cloudnessProp->setAttribute("readOnly", true);
    _browser->addProperty(_cloudnessProp);

    _orbitPathProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Orbit path"));
    _orbitPathProp->setAttribute("readOnly", true);
    _browser->addProperty(_orbitPathProp);

    _orbitRowProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Orbit row"));
    _orbitRowProp->setAttribute("readOnly", true);
    _browser->addProperty(_orbitRowProp);

    _targetPathProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Target path"));
    _targetPathProp->setAttribute("readOnly", true);
    _browser->addProperty(_targetPathProp);

    _targetRowProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Target row"));
    _targetRowProp->setAttribute("readOnly", true);
    _browser->addProperty(_targetRowProp);

    _processingLevelProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Processing level"));
    _processingLevelProp->setAttribute("readOnly", true);
    _browser->addProperty(_processingLevelProp);

    _sunAzimuthProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Sun azimuth"));
    _sunAzimuthProp->setAttribute("readOnly", true);
    _browser->addProperty(_sunAzimuthProp);

    _sunElevationProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Sun elevation"));
    _sunElevationProp->setAttribute("readOnly", true);
    _browser->addProperty(_sunElevationProp);

    _inclinationProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Satellite inclination"));
    _inclinationProp->setAttribute("readOnly", true);
    _browser->addProperty(_inclinationProp);

    _lookAngleProp = _variantManager->addProperty(QVariant::Double, QString::fromUtf8("Look angle"));
    _lookAngleProp->setAttribute("readOnly", true);
    _browser->addProperty(_lookAngleProp);

    _overviewDownloadLabel = new QLabel(QString::fromUtf8("Скачать обзор (<a href='http://google.ru'>ссылка</a>)"));
    _overviewDownloadLabel->setOpenExternalLinks(true);
    layout->addWidget(_overviewDownloadLabel);

    _sceneDownloadLabel = new QLabel(QString::fromUtf8("Скачать сцену (<a href='http://google.ru'>ссылка</a>)"));
    _sceneDownloadLabel->setOpenExternalLinks(true);
    layout->addWidget(_sceneDownloadLabel);

    layout->addStretch();

    connect(&_networkManager, SIGNAL(finished(QNetworkReply*)), SLOT(onFileDownloaded(QNetworkReply*)));
}

void MetadataWidget::setScene(const ScenePtr& scene)
{
    if (_lastScene == scene)
    {
        return;
    }

    _lastScene = scene;

    _sceneidProp->setValue(scene->sceneid);
    _datetimeProp->setValue(scene->sceneTime);

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
            
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    QString overviewFilepath = dataPath + QString("/overviews/") + scene->overviewFilename();

    if (QFile::exists(overviewFilepath))
    {
        std::cout << "Overview exists in the local cache\n";

        makeOverlay(overviewFilepath);
    }
    else
    {
        QNetworkRequest request(QString::fromUtf8("http://earthexplorer.usgs.gov/metadata/1854/%0/").arg(scene->sceneid));
        request.setAttribute(QNetworkRequest::User, QString("Metadata"));
        _networkManager.get(request);
    }
}

void MetadataWidget::onFileDownloaded(QNetworkReply* reply)
{
    std::cout << "Downloaded " << qPrintable(reply->url().url()) << std::endl;;

    QByteArray data = reply->readAll();
    if (!data.isNull() && !data.isEmpty())
    {
        QString requestType = reply->request().attribute(QNetworkRequest::User).toString();

        if (requestType == "Metadata")
        {
            int startIndex = data.indexOf("http://earthexplorer.usgs.gov/browse/eo-1/hyp");
            if (startIndex != -1)
            {
                int endIndex = data.indexOf(".jpeg", startIndex);
                if (endIndex != -1)
                {
                    QByteArray overviewUrlName = data.mid(startIndex, endIndex - startIndex + 5);

                    QNetworkRequest request(QString(overviewUrlName.constData()));
                    request.setAttribute(QNetworkRequest::User, QString("Overview"));
                    _networkManager.get(request);
                }
            }
        }
        else if (requestType == "Overview")
        {
            QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
            QDir dataDir(dataPath);
            if (!dataDir.exists())
            {
                dataDir.mkpath("overviews");
            }

            QString overviewFilepath = dataDir.filePath(QString("overviews/") + reply->url().fileName());

            QFile localFile(overviewFilepath);
            if (!localFile.open(QIODevice::WriteOnly))
            {
                std::cerr << "Failed to open file " << qPrintable(overviewFilepath) << std::endl;
                return;
            }

            localFile.write(data);
            localFile.close();

            if (_lastScene && reply->url().fileName() == _lastScene->overviewFilename())
            {
                makeOverlay(overviewFilepath);
            }
        }
    }

    reply->deleteLater();
}

void MetadataWidget::makeOverlay(const QString& filepath)
{
    if (_mapNode.valid() && _lastScene)
    {
        if (_overlayNode)
        {
            _mapNode->removeChild(_overlayNode);
            _overlayNode = nullptr;
        }

        osg::Image* image = osgDB::readImageFile(filepath.toLocal8Bit().constData());
        if (image)
        {
            ImageOverlay* imageOverlay = new ImageOverlay(_mapNode.get(), image);
            imageOverlay->setLowerLeft(_lastScene->swCorner.x(), _lastScene->swCorner.y());
            imageOverlay->setLowerRight(_lastScene->seCorner.x(), _lastScene->seCorner.y());
            imageOverlay->setUpperRight(_lastScene->neCorner.x(), _lastScene->neCorner.y());
            imageOverlay->setUpperLeft(_lastScene->nwCorner.x(), _lastScene->nwCorner.y());
            imageOverlay->getOrCreateStateSet()->setRenderBinDetails(20, "RenderBin");
            _mapNode->addChild(imageOverlay);

            _overlayNode = imageOverlay;
        }
    }
}