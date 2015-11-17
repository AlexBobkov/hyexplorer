#include "SceneOperationsWidget.hpp"

#include <QSettings>
#include <QMessageBox>

using namespace portal;

SceneOperationsWidget::SceneOperationsWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager)
{
    initUi();
}

SceneOperationsWidget::~SceneOperationsWidget()
{
}

void SceneOperationsWidget::initUi()
{
    _ui.setupUi(this);
    
    QSettings settings;
    _ui.fromSpinBox->setValue(settings.value("SceneOperationsWidget/fromValue", 1).toInt());
    _ui.toSpinBox->setValue(settings.value("SceneOperationsWidget/toValue", 1).toInt());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());

    connect(_ui.fromSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMinimumBandChanged(int)));
    connect(_ui.toSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMaximumBandChanged(int)));
    
    //connect(_ui.selectFragmentButton, SIGNAL(clicked()), this, SIGNAL(selectFragmentRequested()));
    connect(_ui.selectFragmentButton, SIGNAL(clicked()), this, SLOT(selectFragment()));
    connect(_ui.downloadButton, SIGNAL(clicked()), this, SLOT(download()));
}

void SceneOperationsWidget::onMinimumBandChanged(int i)
{
    QSettings settings;
    settings.setValue("SceneOperationsWidget/fromValue", _ui.fromSpinBox->value());

    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());
}

void SceneOperationsWidget::onMaximumBandChanged(int i)
{
    QSettings settings;
    settings.setValue("SceneOperationsWidget/toValue", _ui.toSpinBox->value());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
}

void SceneOperationsWidget::selectFragment()
{
}

void SceneOperationsWidget::download()
{
    emit downloadRequested(_ui.fromSpinBox->value(), _ui.toSpinBox->value());
}

void SceneOperationsWidget::setScene(const ScenePtr& scene)
{
    if (scene->hasScene)
    {
        _ui.statusLabel->setText(tr("Сцена присутствует на нашем сервере\nи доступна для скачивания"));
        _ui.controlWidget->setVisible(true);
    }
    else
    {
        _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере\nи не доступна для работы"));
        _ui.controlWidget->setVisible(false);
    }
}