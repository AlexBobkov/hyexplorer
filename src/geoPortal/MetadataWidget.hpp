#pragma once

#include "Scene.hpp"
#include "DataManager.hpp"

#include <osg/observer_ptr>
#include <osgEarth/MapNode>
#include <osgEarthFeatures/Feature>

#define QT_QTPROPERTYBROWSER_IMPORT
#include <QtVariantPropertyManager>
#include <QtVariantEditorFactory>
#include <QtTreePropertyBrowser>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QLabel>

namespace portal
{
    class MetadataWidget : public QWidget
    {
        Q_OBJECT

    public:
        explicit MetadataWidget(const DataManagerPtr& dataManager, QWidget* parent = 0);
        virtual ~MetadataWidget();
        
    public slots:
        void setScene(const ScenePtr& scene);                    

    private:
        void initUi();

        ScenePtr _scene;

        QtAbstractPropertyBrowser* _browser;
        QtVariantPropertyManager* _variantManager;

        QtVariantProperty* _sceneidProp;
        QtVariantProperty* _datetimeProp;
        QtVariantProperty* _cloudnessProp;
        QtVariantProperty* _orbitPathProp;
        QtVariantProperty* _orbitRowProp;
        QtVariantProperty* _targetPathProp;
        QtVariantProperty* _targetRowProp;

        QtVariantProperty* _processingLevelProp;
        QtVariantProperty* _sunAzimuthProp;
        QtVariantProperty* _sunElevationProp;
        QtVariantProperty* _inclinationProp;
        QtVariantProperty* _lookAngleProp;

        QLabel* _overviewDownloadLabel;
        QLabel* _sceneDownloadLabel;
                                
        DataManagerPtr _dataManager; //Возможно он здесь не нужен
    };
}