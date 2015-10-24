#pragma once

#include <osg/Vec3d>

#include <QDateTime>
#include <QString>

#include <boost/optional.hpp>

#include <memory>

namespace portal
{
    class DataSet
    {
    public:
        void addCondition(const QString& str);

        QString fullCondition() const { return _fullCondition; }

    protected:
        QString _fullCondition;
    };
}