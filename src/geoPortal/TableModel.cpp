#include "TableModel.hpp"

#include <QVariant>

#include <iostream>
#include <vector>

using namespace portal;

TableModel::TableModel(const DataSetPtr& dataset, QObject* parent) :
QAbstractTableModel(parent),
_dataset(dataset)
{
}

int TableModel::rowCount(const QModelIndex& parent) const
{
    return _dataset->scenes().size();
}

int TableModel::columnCount(const QModelIndex& parent) const
{
    return 3;
}

QVariant TableModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        if (index.column() == 0)
        {
            return _dataset->scenes()[index.row()]->sceneid;
        }
        else if (index.column() == 1)
        {
            return _dataset->scenes()[index.row()]->sceneTime.date().toString(Qt::ISODate);
        }
        else if (index.column() == 2 && _dataset->scenes()[index.column()]->cloundMax)
        {
            return _dataset->scenes()[index.row()]->cloundMax.get();
        }

        return QVariant();
    }
    else if (role == Qt::UserRole)
    {
        return QVariant::fromValue(_dataset->scenes()[index.row()]);
    }
    else
    {
        return QVariant();
    }
}

Qt::ItemFlags TableModel::flags(const QModelIndex& index) const
{
    return QAbstractTableModel::flags(index);
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole)
    {
        return QVariant();
    }

    if (orientation == Qt::Vertical)
    {
        return QString::number(section + 1);
    }
    else
    {
        if (section == 0)
        {
            return tr("Scene Id");
        }
        else if (section == 1)
        {
            return tr("Дата");
        }
        else if (section == 2)
        {
            return tr("Макс. облачность");
        }
        
        return QVariant();
    }
}

bool TableModel::setData(const QModelIndex& i, const QVariant& value, int role)
{
    return false;
}