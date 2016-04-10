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

#include "TableModel.hpp"

#include <QVariant>

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
    return 2;
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
            return _dataset->scenes()[index.row()]->sceneId();
        }
        else if (index.column() == 1)
        {
            QVariant datatime = _dataset->scenes()[index.row()]->attrib("scenetime");
            return datatime.toDateTime().date().toString(Qt::ISODate);
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
            return tr("Идентификатор сцены");
        }
        else if (section == 1)
        {
            return tr("Дата");
        }

        return QVariant();
    }
}

bool TableModel::setData(const QModelIndex& i, const QVariant& value, int role)
{
    return false;
}