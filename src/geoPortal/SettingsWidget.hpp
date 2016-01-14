#pragma once

#include "ui_SettingsWidget.h"
#include "DataManager.hpp"

namespace portal
{
    class SettingsWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit SettingsWidget(const DataManagerPtr& dataManager, QWidget* parent = 0);
        virtual ~SettingsWidget();

    private:
        void initUi();

        Ui::SettingsWidget _ui;

        DataManagerPtr _dataManager;
    };
}