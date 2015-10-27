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

    for (const auto& cn : _dataManager->coverageNames())
    {
        _ui.coverageComboBox->addItem(QString::fromUtf8(cn.c_str()));
    }

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
    std::cout << "Change coverage " << qPrintable(text) << std::endl;;

    _dataManager->setCoverage(text.toUtf8().constData());
}

void SettingsWidget::browsePath()
{
    std::cout << "Browser path\n";
}