#pragma once

#include <osgEarthFeatures/Feature>

#define QT_QTPROPERTYBROWSER_IMPORT
#include <QtVariantPropertyManager>
#include <QtVariantEditorFactory>
#include <QtTreePropertyBrowser>

#include <QLabel>

class MetadataWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MetadataWidget();
    virtual ~MetadataWidget();

public slots:
    void setScene(osgEarth::Features::Feature* feature);

private:
    void initUi();

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
};
