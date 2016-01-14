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