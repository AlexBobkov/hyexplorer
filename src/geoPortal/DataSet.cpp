#include "DataSet.hpp"

using namespace portal;

void DataSet::addCondition(const QString& str)
{
    if (_fullCondition.isEmpty())
    {
        _fullCondition = str;
        return;
    }

    _fullCondition += " and " + str;
}