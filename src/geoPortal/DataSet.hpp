#pragma once

#include "Scene.hpp"

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
        Возвращает полное условие выборки сцен из БД
        */
        QString fullCondition() const { return _fullCondition; }

        /**
        Добавляет компонент условия
        */
        void addCondition(const QString& str);

        /**
        Добавляет новое имя сенсора к поиску
        */
        void addSensor(const QString& sensorName);
        
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
        osgEarth::ModelLayer* layer() const { return _layer; }

        /**
        Список всех сцен
        */
        const std::vector<ScenePtr>& scenes() const { return _scenes; }

    protected:
        void selectScenesForHyperion();
        void selectScenesForAviris();

        QString queryForScenesUnderPointerForHyperion(const osgEarth::GeoPoint& point) const;
        QString queryForScenesUnderPointerForAviris(const osgEarth::GeoPoint& point) const;

        bool grabCommonAttributes(const ScenePtr& scene, const QSqlQuery& query, int& column) const;

        typedef std::function<void()> SensorQueryMethodType;
        std::map<QString, SensorQueryMethodType> _sensorQueryMethods;

        typedef std::function<QString(const osgEarth::GeoPoint&)> SensorQueryUnderPointerMethodType;
        std::map<QString, SensorQueryUnderPointerMethodType> _sensorQueryScenesUnderPointerMethods;

        bool _initialized;

        osg::ref_ptr<osgEarth::SpatialReference> _srs;

        QString _fullCondition;
        QStringList _sensors;

        std::vector<ScenePtr> _scenes;

        osg::ref_ptr<osgEarth::Features::FeatureListSource> _featureSource;
        osg::ref_ptr<osgEarth::ModelLayer> _layer;

        std::set<std::size_t> _sceneIdsUnderPointer;
    };

    typedef std::shared_ptr<DataSet> DataSetPtr;
}