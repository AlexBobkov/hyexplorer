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

#pragma once

#include "Scene.hpp"
#include "ClipInfo.hpp"

#include <QString>
#include <QDir>

namespace portal
{
    class Storage
    {
    public:
        static QString overviewPath(const ScenePtr& scene, const QString& filename);

        static QString sceneBandPath(const ScenePtr& scene, int band, const ClipInfoPtr& clipInfo);
        static QString sceneBandPath(const ScenePtr& scene, const QString& filename, const ClipInfoPtr& clipInfo);
        static QDir sceneBandDir(const ScenePtr& scene, const ClipInfoPtr& clipInfo);

        static QString processedFilePath(const ScenePtr& scene, int band, const QString& processedId);
        static QDir processedFileDir(const ScenePtr& scene);
        
        static QString tempPath(const QString& filename);
    };
}
