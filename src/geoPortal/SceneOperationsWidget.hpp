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