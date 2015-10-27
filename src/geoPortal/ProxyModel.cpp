#include "ProxyModel.hpp"

#include <QVariant>

#include <iostream>
#include <vector>

using namespace portal;

ProxyModel::ProxyModel(const DataSetPtr& dataset, QObject* parent) :
QSortFilterProxyModel(parent),
_dataset(dataset)
{
}

bool ProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    ScenePtr scene = index.data(Qt::UserRole).value<ScenePtr>();
    if (!scene)
    {
        std::cerr << "Failed to find scene for row " << sourceRow << std::endl;
        return false;
    }

    return _dataset->isSceneUnderPointer(scene);
}