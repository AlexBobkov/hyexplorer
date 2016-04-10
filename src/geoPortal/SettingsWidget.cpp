/* HyExplorer - hyperspectral images management system
* Copyright (c) 2015-2016 HyExplorer team
* http://virtualglobe.ru/hyexplorer/
*
* Permission is hereby granted, free of charge, to any person obtaining
* a copy of this software and associated documentation files (the
* "Software"), to deal in the Software without restriction, including
* without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to
* permit persons to whom the Software is furnished to do so, subject to
* the following conditions:
*
* The above copyright notice and this permission notice shall be
* included in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
* LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
* OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
* WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

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
