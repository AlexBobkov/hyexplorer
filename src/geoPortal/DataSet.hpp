#pragma once

#include "Scene.hpp"

#include <osgEarth/MapNode>
#include <osgEarthFeatures/FeatureSource>

#include <QString>

#include <memory>

namespace portal
{
    class DataSet
    {
    public:
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
        Выполняет запрос к БД, скачивает все сцены на основе условия _fullCondition
        */
        void selectScenes();

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
        bool _initialized;

        QString _fullCondition;

        std::vector<ScenePtr> _scenes;

        osg::ref_ptr<osgEarth::Features::FeatureSource> _featureSource;
        osg::ref_ptr<osgEarth::ModelLayer> _layer;

        std::set<std::size_t> _sceneIdsUnderPointer;
    };

    typedef std::shared_ptr<DataSet> DataSetPtr;
}