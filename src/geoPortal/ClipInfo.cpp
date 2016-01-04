#include "ClipInfo.hpp"

#include <QDebug>

using namespace portal;

namespace
{
    std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
}

QString portal::genetrateRandomName()
{
    QString name;
    name.reserve(10);

    name.resize(10);
    for (int i = 0; i < name.size(); i++)
    {
        name[i] = alphabet[rand() % alphabet.size()];
    }

    return name;
}

//====================================================

ClipInfo::ClipInfo(const osgEarth::Bounds& bounds) :
_bounds(bounds),
_minBand(0),
_maxBand(0)
{
    _name = genetrateRandomName();

    qDebug() << "Name " << _name;
}