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

#include <QString>
#include <QStringList>
#include <QSqlQuery>

#include <functional>
#include <memory>

namespace portal
{
    class SensorQuery
    {
    public:
        typedef std::function<void(int)> ProgressCallbackType;

        SensorQuery();
        virtual ~SensorQuery();

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
        virtual void selectScenes(std::vector<ScenePtr>& scenes, const ProgressCallbackType& cb = ProgressCallbackType()) = 0;

        /**
        Выполняет запрос к БД, запрашивает какие сцены содержат данную точку
        */
        virtual void selectScenesUnderPointer(std::set<int>& ids, const osgEarth::GeoPoint& point) = 0;

    protected:
        QString _fullCondition;
    };

    typedef std::shared_ptr<SensorQuery> SensorQueryPtr;

    class HyperionQuery : public SensorQuery
    {
        void selectScenes(std::vector<ScenePtr>& scenes, const ProgressCallbackType& cb = ProgressCallbackType()) override;

        void selectScenesUnderPointer(std::set<int>& ids, const osgEarth::GeoPoint& point) override;
    };

    class AvirisQuery : public SensorQuery
    {
        void selectScenes(std::vector<ScenePtr>& scenes, const ProgressCallbackType& cb = ProgressCallbackType()) override;

        void selectScenesUnderPointer(std::set<int>& ids, const osgEarth::GeoPoint& point) override;
    };
}