#include "ClipInfo.hpp"

#include <QDebug>

using namespace portal;

namespace
{
std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";
}

ClipInfo::ClipInfo(const osgEarth::Bounds& bounds) :
_bounds(bounds),
_minBand(0),
_maxBand(0)
{
    _name.resize(10);
    for (int i = 0; i < _name.size(); i++)
    {
        _name[i] = alphabet[rand() % alphabet.size()];
    }

    qDebug() << "Name " << _name;
}