#include "Scene.hpp"

using namespace portal;

Scene::Scene()
{
}

int Scene::id() const
{
    return attrib("ogc_fid").toInt();
}

QString Scene::sensor() const
{
    return attrib("sensor").toString();
}

QString Scene::sceneId() const
{
    return attrib("sceneid").toString();
}

bool Scene::hasOverview() const
{
    return attrib("hasoverview").toBool();
}

bool Scene::hasScene() const
{
    return attrib("hasscene").toBool();
}

void Scene::setSceneExistence(bool b)
{
    setAttrib("hasscene", true);
}

QString Scene::overviewName() const
{
    return attrib("overviewname").toString();
}

QString Scene::sceneUrl() const
{
    return attrib("sceneurl").toString();
}

bool Scene::hasKey(const QString& key) const
{
    return _attribs.find(key) != _attribs.end();
}

QVariant Scene::attrib(const QString& key) const
{
    auto itr = _attribs.find(key);
    if (itr == _attribs.end())
    {
        return QVariant();
    }

    return itr->second;
}
