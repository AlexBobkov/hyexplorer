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

    connect(_ui.atmoCheckBox, &QCheckBox::stateChanged, this, [this](int value)
    {
        _dataManager->setAtmosphereVisibility(value > 0);
    });

    //-----------------------

    for (const auto& cn : _dataManager->coverageNames())
    {
        _ui.coverageComboBox->addItem(cn);
    }

    connect(_ui.coverageComboBox, &QComboBox::currentTextChanged, this, [this](const QString& text)
    {
        _dataManager->setCoverage(text.toUtf8().constData());
    });

    //-----------------------

    QSettings settings;
    _ui.storagePathLineEdit->setText(settings.value("StoragePath").toString());

    connect(_ui.browsePushButton, &QPushButton::clicked, this, [this]()
    {
        QString dirname = QFileDialog::getExistingDirectory(this, tr("Выберите папку для хранилища"), _ui.storagePathLineEdit->text());
        if (!dirname.isNull())
        {
            _ui.storagePathLineEdit->setText(dirname);

            QSettings settings;
            settings.setValue("StoragePath", dirname);

            QMessageBox::information(this, tr("Изменение хранилища"), tr("Новые данные будут размещаться в новой папке. Старые данные останутся в старой. Если вы хотите использовать старые данные, то перенесите их вручную в новую папку."));
        }
    });
}
