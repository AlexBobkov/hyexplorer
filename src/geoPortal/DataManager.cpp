#include "DataManager.hpp"
#include "Storage.hpp"

#include <osg/Image>
#include <osgEarthAnnotation/ImageOverlay>
#include <osgEarth/Viewpoint>
#include <osgEarthUtil/EarthManipulator>
#include <osgEarthAnnotation/CircleNode>
#include <osgEarthFeatures/FeatureSource>
#include <osgEarthFeatures/FeatureDisplayLayout>
#include <osgEarthSymbology/StyleSheet>
#include <osgEarthSymbology/PolygonSymbol>
#include <osgEarthSymbology/AltitudeSymbol>
#include <osgEarthSymbology/LineSymbol>
#include <osgEarthDrivers/model_feature_geom/FeatureGeomModelOptions>
#include <osgEarthDrivers/feature_ogr/OGRFeatureOptions>
#include <osgEarthDrivers/gdal/GDALOptions>
#include <osgEarthDrivers/xyz/XYZOptions>
#include <osgEarthDrivers/arcgis/ArcGISOptions>

#include <QSettings>
#include <QDir>
#include <QDebug>
#include <QFileInfo>

using namespace osgEarth;
using namespace osgEarth::Annotation;
using namespace osgEarth::Drivers;
using namespace osgEarth::Features;
using namespace osgEarth::Symbology;
using namespace osgEarth::Util;
using namespace portal;

namespace
{
    class FixedEphemeris : public Ephemeris
    {
    public:
        FixedEphemeris(osg::Camera* camera) :
            _camera(camera) { }

        osg::Vec3d getSunPositionECEF(const DateTime& dt) const override
        {
            if (!_camera.valid())
            {
                return Ephemeris::getSunPositionECEF(dt);
            }

            osg::Vec3 camPos = osg::Matrix::inverse(_camera->getViewMatrix()).getTrans();
            camPos.normalize();

            return camPos * 149600000;
        }

    protected:
        osg::observer_ptr<osg::Camera> _camera;
    };

    struct AnimateSkyUpdateCallback : public osg::NodeCallback
    {
        void operator()(osg::Node* node, osg::NodeVisitor* nv) override
        {
            SkyNode* sky = dynamic_cast<SkyNode*>(node);
            if (sky)
            {
                sky->setDateTime(sky->getDateTime());
            }

            traverse(node, nv);
        }
    };
}

DataManager::DataManager(osgViewer::View* view, osgEarth::MapNode* mapNode) :
_view(view),
_mapNode(mapNode)
{
    {
        GDALOptions sourceOpt;
        sourceOpt.url() = "data/world.tif";

        ImageLayerOptions imageOpt;
        imageOpt.driver() = sourceOpt;

        _coverageMap["Low res"] = imageOpt;
        _coverageNames.push_back("Low res");
    }

    {
        XYZOptions sourceOpt;
        sourceOpt.url() = "http://[abc].tile.openstreetmap.org/{z}/{x}/{y}.png";
        sourceOpt.profile() = ProfileOptions("spherical-mercator");

        ImageLayerOptions imageOpt;
        imageOpt.driver() = sourceOpt;
        imageOpt.cachePolicy() = CachePolicy::NO_CACHE;

        _coverageMap["OpenStreetMap"] = imageOpt;
        _coverageNames.push_back("OpenStreetMap");
    }

    QSettings settings;
    if (settings.contains("Rectangle/xMin") &&
        settings.contains("Rectangle/xMax") &&
        settings.contains("Rectangle/yMin") &&
        settings.contains("Rectangle/yMax"))
    {
        _bounds = osgEarth::Bounds(settings.value("Rectangle/xMin").toDouble(), settings.value("Rectangle/yMin").toDouble(), settings.value("Rectangle/xMax").toDouble(), settings.value("Rectangle/yMax").toDouble());
    }
}

void DataManager::addReportHandler(osgGA::GUIEventHandler* handler)
{
    _view->addEventHandler(handler);
}

void DataManager::removeReportHandler(osgGA::GUIEventHandler* handler)
{
    _view->removeEventHandler(handler);
}

void DataManager::setDefaultActionHandler(osgGA::GUIEventHandler* handler)
{
    if (_defaultHandler == handler)
    {
        return;
    }

    if (_defaultHandler)
    {
        _view->removeEventHandler(_defaultHandler);
    }

    _defaultHandler = handler;

    if (_defaultHandler)
    {
        _view->addEventHandler(_defaultHandler);
    }
}

void DataManager::setActionHandler(osgGA::GUIEventHandler* handler)
{
    if (_currentHandler == handler)
    {
        return;
    }

    if (_currentHandler)
    {
        _view->removeEventHandler(_currentHandler);
        _view->addEventHandler(_defaultHandler);
    }

    _currentHandler = handler;

    if (_currentHandler)
    {
        _view->removeEventHandler(_defaultHandler);
        _view->addEventHandler(_currentHandler);
    }
}

void DataManager::setDataSet(const DataSetPtr& dataset)
{
    if (_dataset)
    {
        _mapNode->getMap()->removeModelLayer(_dataset->getOrCreateLayer());
    }

    _dataset = dataset;

    if (_dataset)
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();

        _mapNode->getMap()->addModelLayer(_dataset->getOrCreateLayer());

        osg::Timer_t endTick = osg::Timer::instance()->tick();
        qDebug() << "Loading time " << osg::Timer::instance()->delta_s(startTick, endTick);
    }
}

void DataManager::setCircleNode(const osgEarth::GeoPoint& center, double radius)
{
    removeCircleNode();

    Style circleStyle;
    circleStyle.getOrCreate<LineSymbol>()->stroke()->color() = Color(Color::Red, 1.0);
    circleStyle.getOrCreate<LineSymbol>()->stroke()->width() = 4.0f;
    circleStyle.getOrCreate<AltitudeSymbol>()->clamping() = AltitudeSymbol::CLAMP_TO_TERRAIN;
    circleStyle.getOrCreate<AltitudeSymbol>()->technique() = AltitudeSymbol::TECHNIQUE_DRAPE;
    circleStyle.getOrCreate<RenderSymbol>()->lighting() = false;

    _circleNode = new osgEarth::Annotation::CircleNode(_mapNode,
                                                       center,
                                                       Linear(radius, Units::METERS),
                                                       circleStyle);
    _mapNode->addChild(_circleNode);
}

void DataManager::removeCircleNode()
{
    if (_circleNode)
    {
        _mapNode->removeChild(_circleNode);
    }
}

void DataManager::zoomToScene(const ScenePtr& scene)
{
    if (!scene)
    {
        return;
    }

    osg::Vec3d center = scene->geometry()->getBounds().center();//(scene->neCorner + scene->nwCorner + scene->seCorner + scene->swCorner) * 0.25;

    osgEarth::Viewpoint viewpoint;
    viewpoint.focalPoint() = GeoPoint(_mapNode->getMapSRS(), center.x(), center.y(), 0.0, osgEarth::ALTMODE_ABSOLUTE);
    viewpoint.heading() = 0.0;
    viewpoint.pitch() = 10.0 - 90.0;
    viewpoint.range() = 150000.0;

    EarthManipulator* em = dynamic_cast<EarthManipulator*>(_view->getCameraManipulator());
    if (!em)
    {
        qDebug() << "Failed to cast to EarthManipulator";
        return;
    }

    em->setViewpoint(viewpoint, 3.0);
}

bool DataManager::atmosphereVisibility() const
{
    return _sky.valid();
}

void DataManager::setAtmosphereVisibility(bool b)
{
    if (_sky)
    {
        osg::Group* parent = _sky->getParent(0);
        parent->addChild(_mapNode);
        _sky->removeChild(_mapNode);
        parent->removeChild(_sky);
        _sky = 0;
    }
    else
    {
        SkyOptions options;
        options.setDriver("simple");

        _sky = SkyNode::create(options, _mapNode);
        if (_sky)
        {
            _sky->attach(_view, 0);
            _sky->setEphemeris(new FixedEphemeris(_view->getCamera()));

            osg::Group* parent = _mapNode->getParent(0);
            parent->addChild(_sky);
            _sky->addChild(_mapNode);
            parent->removeChild(_mapNode);

            _sky->setUpdateCallback(new AnimateSkyUpdateCallback());
        }
    }
}

void DataManager::setCoverage(const QString& coverageName)
{
    if (_coverageMap.find(coverageName) == _coverageMap.end())
    {
        qDebug() << "Failed to find coverage " << coverageName;
        return;
    }

    while (_mapNode->getMap()->getNumImageLayers() > 0)
    {
        _mapNode->getMap()->removeImageLayer(_mapNode->getMap()->getImageLayerAt(0));
    }

    ImageLayerOptions opt = _coverageMap[coverageName];
    ImageLayer* layer = new ImageLayer(coverageName.toUtf8().constData(), opt);
    _mapNode->getMap()->addImageLayer(layer);
}

void DataManager::showOverview(const ScenePtr& scene, const QString& filepath)
{
    if (_overlayNode)
    {
        _mapNode->removeChild(_overlayNode);
        _overlayNode = nullptr;
    }

    if (scene->geometry()->getType() == Geometry::TYPE_POLYGON && scene->geometry()->size() >= 4)
    {
        osg::Image* image = osgDB::readImageFile(filepath.toLocal8Bit().constData());
        if (image)
        {
            osg::Vec3d swCorner = scene->geometry()->at(3);
            osg::Vec3d seCorner = scene->geometry()->at(0);
            osg::Vec3d neCorner = scene->geometry()->at(1);
            osg::Vec3d nwCorner = scene->geometry()->at(2);

            ImageOverlay* imageOverlay = new ImageOverlay(_mapNode, image);
            imageOverlay->setLowerLeft(swCorner.x(), swCorner.y());
            imageOverlay->setLowerRight(seCorner.x(), seCorner.y());
            imageOverlay->setUpperRight(neCorner.x(), neCorner.y());
            imageOverlay->setUpperLeft(nwCorner.x(), nwCorner.y());
            imageOverlay->getOrCreateStateSet()->setRenderBinDetails(20, "RenderBin");
            _mapNode->addChild(imageOverlay);

            _overlayNode = imageOverlay;
        }
    }
}

void DataManager::showScene(const ScenePtr& scene, int band, const ClipInfoPtr& clipInfo)
{
    _scene = scene;

    if (!_scene)
    {
        return;
    }

    QString filepath = Storage::sceneBandPath(_scene, band, clipInfo);
    if (!QFileInfo::exists(filepath))
    {
        qDebug() << "File is not found " << filepath;
        return;
    }

    qDebug() << "Show band " << filepath;

    GDALOptions sourceOpt;
    sourceOpt.url() = filepath.toLocal8Bit().constData();

    ImageLayerOptions imageOpt;
    imageOpt.driver() = sourceOpt;

    if (_sceneLayer)
    {
        _mapNode->getMap()->removeImageLayer(_sceneLayer);
    }

    _sceneLayer = new ImageLayer(scene->sceneId().toUtf8().constData(), imageOpt);
    _mapNode->getMap()->addImageLayer(_sceneLayer);

    if (_overlayNode)
    {
        _mapNode->removeChild(_overlayNode);
        _overlayNode = nullptr;
    }
}

void DataManager::setBounds(const osgEarth::Bounds& b)
{
    _bounds = b;

    QSettings settings;
    settings.setValue("Rectangle/xMin", _bounds->xMin());
    settings.setValue("Rectangle/xMax", _bounds->xMax());
    settings.setValue("Rectangle/yMin", _bounds->yMin());
    settings.setValue("Rectangle/yMax", _bounds->yMax());
}