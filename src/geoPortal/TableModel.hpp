#pragma once

#include "DataSet.hpp"

#include <QAbstractTableModel>

namespace portal
{
    class TableModel : public QAbstractTableModel
    {
        Q_OBJECT

    public:
        explicit TableModel(const DataSetPtr& dataset, QObject* parent = 0);

        int rowCount(const QModelIndex& parent = QModelIndex()) const override;
        int columnCount(const QModelIndex& parent = QModelIndex()) const override;
        QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
        Qt::ItemFlags flags(const QModelIndex& index) const override;
        QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

        bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override;

    private:
        DataSetPtr _dataset;
    };
}