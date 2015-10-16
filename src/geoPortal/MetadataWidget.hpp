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
        explicit MetadataWidget();
        virtual ~MetadataWidget();

        void setMapNode(osgEarth::MapNode* mapNode) { _mapNode = mapNode; }

    public slots:
        void setScene(const ScenePtr& scene);

    private slots:
        void onFileDownloaded(QNetworkReply* reply);

    private:
        void initUi();

        void makeOverlay(const QString& filepath);

        osg::observer_ptr<osgEarth::MapNode> _mapNode;
        osg::ref_ptr<osg::Node> _overlayNode;

        ScenePtr _lastScene;

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

        QNetworkAccessManager _networkManager;
    };
}