#pragma once

#include "DataSet.hpp"

#include <QSortFilterProxyModel>
#include <QObject>

namespace portal
{
    class ProxyModel : public QSortFilterProxyModel
    {
        Q_OBJECT

    public:
        explicit ProxyModel(const DataSetPtr& dataset, QObject* parent = 0);

    protected:
        bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

    private:
        DataSetPtr _dataset;
    };
}