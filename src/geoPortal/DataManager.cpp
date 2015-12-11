#include "DataManager.hpp"

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
_mapNode(mapNode),
_activeBand(1),
_clipMode(false)
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
}

void DataManager::setDataSet(const DataSetPtr& dataset)
{
    if (_dataset)
    {
        _mapNode->getMap()->removeModelLayer(_dataset->layer());
        _dataset = nullptr;
    }

    _dataset = dataset;

    if (_dataset)
    {
        osg::Timer_t startTick = osg::Timer::instance()->tick();

        _mapNode->getMap()->addModelLayer(_dataset->layer());

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

    osg::Vec3d center = (scene->neCorner + scene->nwCorner + scene->seCorner + scene->swCorner) * 0.25;

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

    osg::Image* image = osgDB::readImageFile(filepath.toLocal8Bit().constData());
    if (image)
    {
        ImageOverlay* imageOverlay = new ImageOverlay(_mapNode, image);
        imageOverlay->setLowerLeft(scene->swCorner.x(), scene->swCorner.y());
        imageOverlay->setLowerRight(scene->seCorner.x(), scene->seCorner.y());
        imageOverlay->setUpperRight(scene->neCorner.x(), scene->neCorner.y());
        imageOverlay->setUpperLeft(scene->nwCorner.x(), scene->nwCorner.y());
        imageOverlay->getOrCreateStateSet()->setRenderBinDetails(20, "RenderBin");
        _mapNode->addChild(imageOverlay);

        _overlayNode = imageOverlay;
    }
}

void DataManager::setActiveBand(int band)
{
    qDebug() << "Set active band " << _activeBand;

    _activeBand = band;
    showScene(_scene);
}

void DataManager::setClipMode(bool b)
{
    _clipMode = b;
    showScene(_scene);
}

void DataManager::showScene(const ScenePtr& scene)
{
    _scene = scene;

    if (!_scene)
    {
        return;
    }

    QSettings settings;
    QString dataPath = settings.value("StoragePath").toString();
        
    QString filepath;
    if (_clipMode)
    {
        QDir dir(QString("%0/hyperion/clips/%1/").arg(dataPath).arg(scene->sceneId));
        QStringList entries = dir.entryList(QDir::AllEntries | QDir::NoDotAndDotDot);

        int clipNumber = entries.size() - 1;
        filepath = QString("%0/hyperion/clips/%1/clip%2/%3B%4_L1T_clip.TIF").arg(dataPath).arg(scene->sceneId).arg(clipNumber).arg(scene->sceneId.mid(0, 23)).arg(_activeBand, 3, 10, QChar('0'));
    }
    else
    {
        filepath = QString("%0/hyperion/scenes/%1/%2B%3_L1T.TIF").arg(dataPath).arg(scene->sceneId).arg(scene->sceneId.mid(0, 23)).arg(_activeBand, 3, 10, QChar('0'));
    }

    qDebug() << "Show band " << filepath;

    if (!QFileInfo::exists(filepath))
    {
        qDebug() << "File is not found " << filepath;
        return;
    }

    GDALOptions sourceOpt;
    sourceOpt.url() = filepath.toLocal8Bit().constData();

    ImageLayerOptions imageOpt;
    imageOpt.driver() = sourceOpt;

    if (_sceneLayer)
    {
        _mapNode->getMap()->removeImageLayer(_sceneLayer);
    }

    _sceneLayer = new ImageLayer(scene->sceneId.toUtf8().constData(), imageOpt);
    _mapNode->getMap()->addImageLayer(_sceneLayer);

    //if (_overlayNode)
    //{
    //    _mapNode->removeChild(_overlayNode);
    //    _overlayNode = nullptr;
    //}
}