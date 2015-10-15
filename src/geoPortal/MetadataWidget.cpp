#include "MetadataWidget.hpp"

#include <QVBoxLayout>

#include <iostream>

MetadataWidget::MetadataWidget() :
QWidget()
{
    initUi();
}

MetadataWidget::~MetadataWidget()
{
}

void MetadataWidget::initUi()
{
    QVBoxLayout* layout = new QVBoxLayout;
    layout->setMargin(0);
    setLayout(layout);

    QLabel* label = new QLabel(QString::fromUtf8("Тест"));
    layout->addWidget(label);

    layout->addStretch(1);
}

void MetadataWidget::setSceneId(const QString& sceneid)
{    
}