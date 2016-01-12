#include "ClipInfo.hpp"
#include "Utils.hpp"

#include <QDebug>

using namespace portal;

ClipInfo::ClipInfo(const ScenePtr& scene) :
_scene(scene),
_minBand(0),
_maxBand(0)
{

}

ClipInfo::ClipInfo(const ScenePtr& scene, const osgEarth::Bounds& bounds) :
_scene(scene),
_minBand(0),
_maxBand(0),
_bounds(bounds),
_uniqueName(genetrateRandomName())
{
}