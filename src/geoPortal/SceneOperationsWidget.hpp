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

#include "ui_SceneOperationsWidget.h"
#include "DataManager.hpp"
#include "EventHandlers.hpp"

#include <QWidget>
#include <QProcess>

namespace portal
{
    class SceneOperationsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit SceneOperationsWidget(const DataManagerPtr& dataManager, QWidget* parent = 0);
        virtual ~SceneOperationsWidget();

    signals:
        void sceneClipPrepared(const ScenePtr& scene, const ClipInfoPtr&);

        void progressChanged(int);
        void progressReset();

    public slots:
        void setScene(const ScenePtr& scene);

    private:
        void initUi();

        Ui::SceneOperationsWidget _ui;

        DataManagerPtr _dataManager;

        osg::ref_ptr<DrawRectangleMouseHandler> _drawRectangleHandler;

        ScenePtr _scene;
        ClipInfoPtr _clipInfo;

        bool _importing;
    };
}