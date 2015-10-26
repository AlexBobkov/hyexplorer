﻿#pragma once

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
        QString fullCondition() const { return _fullCondition; }
        void addCondition(const QString& str);
        
        /**
        Выполняет запрос к БД, скачивает все сцены на основе условия _fullCondition
        */
        void selectScenes();

        /**
        Выполняет запрос к БД, запрашивает какие сцены содержат данную точку
        */
        void selectScenesUnderPointer(const osgEarth::GeoPoint& point);

        bool isSceneUnderPointer(const ScenePtr& scene) const;

        osgEarth::ModelLayer* layer() const { return _layer; }

        const std::vector<ScenePtr>& scenes() const { return _scenes; }

    protected:
        QString _fullCondition;

        std::vector<ScenePtr> _scenes;

        osg::ref_ptr<osgEarth::Features::FeatureSource> _featureSource;
        osg::ref_ptr<osgEarth::ModelLayer> _layer;

        std::set<std::size_t> _sceneIdsUnderPointer;
    };

    typedef std::shared_ptr<DataSet> DataSetPtr;
}