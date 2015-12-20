#pragma once

#include "Scene.hpp"
#include "SensorQuery.hpp"

#include <osgEarth/MapNode>
#include <osgEarthFeatures/FeatureSource>
#include <osgEarthFeatures/FeatureListSource>

#include <QString>
#include <QStringList>
#include <QSqlQuery>

#include <functional>
#include <memory>

namespace portal
{
    class DataSet
    {
    public:
        typedef std::function<void(int)> ProgressCallbackType;

        DataSet();

        bool isInitialized() const { return _initialized; }

        /**
        Добавляет новое имя сенсора к поиску
        */
        void addSensor(const SensorQueryPtr& sensorQuery);
                        
        /**
        Выполняет запрос к БД, скачивает все сцены на основе условия _fullCondition
        */
        void selectScenes(const ProgressCallbackType& cb = ProgressCallbackType());

        /**
        Выполняет запрос к БД, запрашивает какие сцены содержат данную точку
        */
        void selectScenesUnderPointer(const osgEarth::GeoPoint& point);

        /**
        Возвращает истину, если данная сцена находится под указателем
        */
        bool isSceneUnderPointer(const ScenePtr& scene) const;

        /**
        Слой для отображения на глобусе
        */
        osg::ref_ptr<osgEarth::ModelLayer> getOrCreateLayer();

        /**
        Список всех сцен
        */
        const std::vector<ScenePtr>& scenes() const { return _scenes; }

    protected:
        bool _initialized;        

        std::vector<SensorQueryPtr> _sensors;

        std::vector<ScenePtr> _scenes;
        std::set<std::size_t> _sceneIdsUnderPointer;

        osg::ref_ptr<osgEarth::SpatialReference> _srs;
        osg::ref_ptr<osgEarth::ModelLayer> _layer;        
    };

    typedef std::shared_ptr<DataSet> DataSetPtr;
}