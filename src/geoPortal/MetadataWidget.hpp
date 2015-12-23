#pragma once

#include "Scene.hpp"

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
        explicit MetadataWidget(QWidget* parent = 0);
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
        QtVariantProperty* _pixelSizeProp;
        QtVariantProperty* _sunAzimuthProp;
        QtVariantProperty* _sunElevationProp;        
        
        //-- Hyperion

        QtVariantProperty* _cloudnessProp;
        QtVariantProperty* _orbitPathProp;
        QtVariantProperty* _orbitRowProp;
        QtVariantProperty* _targetPathProp;
        QtVariantProperty* _targetRowProp;
        QtVariantProperty* _processingLevelProp;        
        QtVariantProperty* _inclinationProp;
        QtVariantProperty* _lookAngleProp;

        //-- AVIRIS

        QtVariantProperty* _sitenameProp;
        QtVariantProperty* _commentsProp;
        QtVariantProperty* _investigatorProp;
        QtVariantProperty* _scenerotationProp;
        QtVariantProperty* _tapeProp;
        QtVariantProperty* _geoverProp;
        QtVariantProperty* _rdnverProp;
        QtVariantProperty* _meanSceneElevProp;
        QtVariantProperty* _minSceneElevProp;
        QtVariantProperty* _maxSceneElevProp;
        QtVariantProperty* _flightProp;
        QtVariantProperty* _runProp;

        //QLabel* _overviewDownloadLabel;
        //QLabel* _sceneDownloadLabel;
                                
        //DataManagerPtr _dataManager; //Возможно он здесь не нужен
    };
}