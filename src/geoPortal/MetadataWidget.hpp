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

        std::map<QString, QtVariantProperty*> _props;
        std::map<QString, QString> _translations;
    };
}