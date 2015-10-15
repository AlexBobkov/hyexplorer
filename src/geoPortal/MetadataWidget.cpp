#include "MetadataWidget.hpp"

#include <QVBoxLayout>
#include <QDateTime>

#include <iostream>

MetadataWidget::MetadataWidget() :
QWidget(),
_browser(0),
_variantManager(0),
_sceneidProp(0),
_datetimeProp(0),
_cloudnessProp(0),
_orbitPathProp(0),
_orbitRowProp(0),
_targetPathProp(0),
_targetRowProp(0),
_processingLevelProp(0),
_sunAzimuthProp(0),
_sunElevationProp(0),
_inclinationProp(0),
_lookAngleProp(0)
{
    initUi();
}

MetadataWidget::~MetadataWidget()
{
}

void MetadataWidget::initUi()
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setMargin(0);
    setLayout(layout);

    _variantManager = new QtVariantPropertyManager(this);
    
    _browser = new QtTreePropertyBrowser(this);
    _browser->setFactoryForManager(_variantManager, new QtVariantEditorFactory(this));    
    layout->addWidget(_browser);

    _sceneidProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Scene Id"));
    _sceneidProp->setAttribute("readOnly", true);
    _browser->addProperty(_sceneidProp);

    _datetimeProp = _variantManager->addProperty(QVariant::DateTime, QString::fromUtf8("Scene time"));
    _datetimeProp->setAttribute("readOnly", true);
    _browser->addProperty(_datetimeProp);

    _cloudnessProp = _variantManager->addProperty(QVariant::String, QString::fromUtf8("Cloud cover"));
    _cloudnessProp->setAttribute("readOnly", true);
    _browser->addProperty(_cloudnessProp);

    _orbitPathProp = _variantManager->addProperty(QVariant::Int, QString::fromUtf8("Orbit path"));
    _orbitPathProp->setAttribute("readOnly", true);
    _browser->addProperty(_orbitPathProp);
    

#if 0
    QLabel* label = new QLabel(QString::fromUtf8("Тест"));
    layout->addWidget(label);
#endif

    layout->addStretch(1);
}

void MetadataWidget::setScene(osgEarth::Features::Feature* feature)
{    
    std::string sceneid = feature->getString("sceneid");
    _sceneidProp->setValue(sceneid.c_str());

    std::string scenetime = feature->getString("scenetime");
    _datetimeProp->setValue(QDateTime::fromString(scenetime.c_str(), Qt::ISODate));

    int cloudMin = feature->getInt("cloudmin");
    int cloudMax = feature->getInt("cloudmax");
    _cloudnessProp->setValue(QString("%0% to %1%").arg(cloudMin).arg(cloudMax));

    int orbitPath = feature->getInt("orbitpath");
    _orbitPathProp->setValue(orbitPath);
}