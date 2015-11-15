#include "DownloadWidget.hpp"

#include <QSettings>
#include <QMessageBox>

using namespace portal;

DownloadWidget::DownloadWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager)
{
    initUi();
}

DownloadWidget::~DownloadWidget()
{
}

void DownloadWidget::initUi()
{
    _ui.setupUi(this);
    
    QSettings settings;
    _ui.fromSpinBox->setValue(settings.value("DownloadWidget/fromValue", 1).toInt());
    _ui.toSpinBox->setValue(settings.value("DownloadWidget/toValue", 1).toInt());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());

    connect(_ui.fromSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMinimumBandChanged(int)));
    connect(_ui.toSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMaximumBandChanged(int)));
    
    //connect(_ui.selectFragmentButton, SIGNAL(clicked()), this, SIGNAL(selectFragmentRequested()));
    connect(_ui.selectFragmentButton, SIGNAL(clicked()), this, SLOT(selectFragment()));
    connect(_ui.downloadButton, SIGNAL(clicked()), this, SLOT(download()));
}

void DownloadWidget::onMinimumBandChanged(int i)
{
    QSettings settings;
    settings.setValue("DownloadWidget/fromValue", _ui.fromSpinBox->value());

    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());
}

void DownloadWidget::onMaximumBandChanged(int i)
{
    QSettings settings;
    settings.setValue("DownloadWidget/toValue", _ui.toSpinBox->value());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
}

void DownloadWidget::selectFragment()
{
}

void DownloadWidget::download()
{
    emit downloadRequested();
}