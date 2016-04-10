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
