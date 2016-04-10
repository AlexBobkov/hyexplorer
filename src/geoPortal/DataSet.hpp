/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
        std::set<int> _sceneIdsUnderPointer;

        osg::ref_ptr<osgEarth::SpatialReference> _srs;
        osg::ref_ptr<osgEarth::ModelLayer> _layer;        
    };

    typedef std::shared_ptr<DataSet> DataSetPtr;
}