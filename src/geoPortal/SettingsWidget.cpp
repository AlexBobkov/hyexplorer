#include "SettingsWidget.hpp"

#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>

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
        _ui.coverageComboBox->addItem(cn);
    }

    QSettings settings;
    _ui.storagePathLineEdit->setText(settings.value("StoragePath").toString());

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
    _dataManager->setCoverage(text.toUtf8().constData());
}

void SettingsWidget::browsePath()
{    
    QString dirname = QFileDialog::getExistingDirectory(this, tr("Выберите папку для хранилища"), _ui.storagePathLineEdit->text());
    if (!dirname.isNull())
    {
        _ui.storagePathLineEdit->setText(dirname);

        QSettings settings;
        settings.setValue("StoragePath", dirname);

        QMessageBox::information(this, tr("Изменение хранилища"), tr("Новые данные будут размещаться в новой папке. Старые данные останутся в старой. Если вы хотите использовать старые данные, то перенесите их вручную в новую папку."));
    }
}