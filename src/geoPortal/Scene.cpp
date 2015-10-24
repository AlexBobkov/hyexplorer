#include "Scene.hpp"

using namespace portal;

QString Scene::overviewFilename() const
{
    QString overviewFilename = QString("%0.jpeg").arg(sceneid);
    overviewFilename.replace("EO1H", "EO1"); //Hyperion scecific
    return overviewFilename;
}