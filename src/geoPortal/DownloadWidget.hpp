#pragma once

#include "ui_DownloadWidget.h"
#include "DataManager.hpp"

#include <QWidget>

namespace portal
{
    class DownloadWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit DownloadWidget(const DataManagerPtr& dataManager, QWidget* parent = 0);        
        virtual ~DownloadWidget();

    signals:
        void downloadRequested();
        //void selectFragmentRequested();
        
    private slots:
        void onMinimumBandChanged(int i);
        void onMaximumBandChanged(int i);
        void selectFragment();
        void download();
    
    private:
        void initUi();

        Ui::DownloadWidget _ui;

        DataManagerPtr _dataManager;
    };
}