#include "SettingsWidget.hpp"

using namespace portal;

SettingsWidget::SettingsWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager)
{
    initUi();
}

SettingsWidget::~SettingsWidget()
{
}

void SettingsWidget::initUi()
{
    _ui.setupUi(this);

    _ui.atmoCheckBox->setChecked(_dataManager->atmosphereVisibility());

    connect(_ui.atmoCheckBox, SIGNAL(stateChanged(int)), this, SLOT(changeAtmoVisibility(int)));
    connect(_ui.coverageComboBox, SIGNAL(currentTextChanged(const QString&)), this, SLOT(changeCoverage(const QString&)));

    connect(_ui.browsePushButton, SIGNAL(clicked()), this, SLOT(browsePath()));
}

void SettingsWidget::changeAtmoVisibility(int value)
{
    _dataManager->setAtmosphereVisibility(value > 0);
}

void SettingsWidget::changeCoverage(const QString& text)
{
    std::cout << "Change coverage\n";
}

void SettingsWidget::browsePath()
{
    std::cout << "Browser path\n";
}