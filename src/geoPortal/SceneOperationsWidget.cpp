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

#include "SceneOperationsWidget.hpp"
#include "Storage.hpp"
#include "Utils.hpp"
#include "Operations.hpp"

#include <QDebug>
#include <QSettings>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QFileInfo>
#include <QSqlQueryModel>
#include <QTableView>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QHttpMultiPart>

using namespace portal;

SceneOperationsWidget::SceneOperationsWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager),
_importing(false)
{
    initUi();
}

SceneOperationsWidget::~SceneOperationsWidget()
{
}

void SceneOperationsWidget::initUi()
{
    _ui.setupUi(this);

    _ui.importButton->setVisible(false);
    _ui.bandsGroupBox->setEnabled(false);
    _ui.boundsGroupBox->setEnabled(false);
    _ui.downloadButton->setEnabled(false);
    _ui.openFolderButton->setEnabled(false);

    //---------------------------------------

    connect(_ui.importButton, &QPushButton::clicked, this, [this]()
    {
        QMessageBox::information(qApp->activeWindow(), tr("Импорт сцены"), tr("Импорт сцены отключен из-за нехватки свободного места"));
        return;

        _importing = true;

        _ui.importButton->setEnabled(!_importing);

        ImportSceneOperation* op = new ImportSceneOperation(_scene, &_dataManager->networkAccessManager(), this);

        connect(op, &ImportSceneOperation::progressChanged, this, &SceneOperationsWidget::progressChanged);

        connect(op, &ImportSceneOperation::finished, this, [op, this](const ScenePtr& scene)
        {
            op->deleteLater();
            op->setParent(0);

            QMessageBox::information(qApp->activeWindow(), tr("Импорт сцены"), tr("Сцена %0 успешно получена с сервера USGS и загружена на наш сервер").arg(scene->sceneId()));

            _importing = false;

            scene->setSceneExistence(true);

            if (_scene == scene) //reload widget
            {
                setScene(ScenePtr());
                setScene(scene);
            }

            emit progressReset();
        });

        connect(op, &ImportSceneOperation::error, this, [op, this](const QString& text)
        {
            op->deleteLater();
            op->setParent(0);

            QMessageBox::warning(qApp->activeWindow(), tr("Импорт сцены"), text);

            _importing = false;

            emit progressReset();
        });

        op->start();
    });

    //---------------------------------------

    QSettings settings;
    _ui.fromSpinBox->setValue(settings.value("SceneOperationsWidget/fromValue", 1).toInt());
    _ui.toSpinBox->setValue(settings.value("SceneOperationsWidget/toValue", 1).toInt());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());

    connect(_ui.fromSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("SceneOperationsWidget/fromValue", _ui.fromSpinBox->value());

        _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());
    });

    connect(_ui.toSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("SceneOperationsWidget/toValue", _ui.toSpinBox->value());

        _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    });

    //---------------------------------------

    _drawRectangleHandler = new DrawRectangleMouseHandler(_dataManager->mapNode(),
                                                          [this](const osgEarth::Bounds& b)
    {
        QMetaObject::invokeMethod(_ui.selectFragmentButton, "setChecked", Qt::QueuedConnection, Q_ARG(bool, false));

        _ui.leftSpinBox->setMaximum(b.xMax());
        _ui.rightSpinBox->setMinimum(b.xMin());
        _ui.topSpinBox->setMinimum(b.yMin());
        _ui.bottomSpinBox->setMaximum(b.yMax());

        _ui.leftSpinBox->setValue(b.xMin());
        _ui.rightSpinBox->setValue(b.xMax());
        _ui.topSpinBox->setValue(b.yMax());
        _ui.bottomSpinBox->setValue(b.yMin());

        _dataManager->setBounds(b);
    },
        std::bind(&DataManager::drawBounds, _dataManager, std::placeholders::_1),
        [this]()
    {
        QMetaObject::invokeMethod(_ui.selectFragmentButton, "setChecked", Qt::QueuedConnection, Q_ARG(bool, false));
    });

    connect(_ui.selectFragmentButton, &QPushButton::toggled, this, [this](bool b)
    {
        if (b)
        {
            _drawRectangleHandler->reset();
            _dataManager->setActionHandler(_drawRectangleHandler);
        }
        else
        {            
            _dataManager->setActionHandler(0);
            _dataManager->drawBounds(*_dataManager->bounds()); //return to old bounds
        }
    });

    //---------------------------------------

    connect(_ui.downloadButton, &QPushButton::clicked, this, [this]()
    {
        if (_ui.fragmentRadioButton->isChecked() && !_dataManager->bounds())
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Сначала выделите фрагмент"));
            return;
        }

        if (_ui.fragmentRadioButton->isChecked() && !_dataManager->bounds()->intersects(_scene->geometry()->getBounds()))
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Фрагмент не пересекает границы сцены"));
            return;
        }

        setEnabled(false);

        if (_ui.fragmentRadioButton->isChecked())
        {
            _clipInfo = std::make_shared<ClipInfo>(_scene, *_dataManager->bounds());
        }
        else
        {
            _clipInfo = std::make_shared<ClipInfo>(_scene);
        }
        _clipInfo->setMinBand(_ui.fromSpinBox->value());
        _clipInfo->setMaxBand(_ui.toSpinBox->value());

        DownloadSceneOperation* op = new DownloadSceneOperation(_scene, _clipInfo, &_dataManager->networkAccessManager(), this);

        connect(op, &DownloadSceneOperation::progressChanged, this, &SceneOperationsWidget::progressChanged);

        connect(op, &DownloadSceneOperation::finished, this, [op, this](const ScenePtr& scene, const ClipInfoPtr& clipInfo)
        {
            op->deleteLater();
            op->setParent(0);

            QMessageBox::information(qApp->activeWindow(), tr("Скачивание сцены"), tr("Выбранные каналы скачаны"));

            setEnabled(true);

            _ui.openFolderButton->setEnabled(true);
            openExplorer(Storage::sceneBandDir(scene, clipInfo).path());

            emit sceneClipPrepared(scene, clipInfo);
        });

        connect(op, &DownloadSceneOperation::error, this, [op, this](const QString& text)
        {
            op->deleteLater();
            op->setParent(0);

            QMessageBox::warning(qApp->activeWindow(), tr("Скачивание сцены"), text);

            setEnabled(true);
        });

        op->start();
    });

    connect(_ui.openFolderButton, &QPushButton::clicked, this, [this]()
    {
        openExplorer(Storage::sceneBandDir(_scene, _clipInfo).path());
    });

    //---------------------------------------

    boost::optional<osgEarth::Bounds> bounds = _dataManager->bounds();
    if (bounds)
    {
        _ui.leftSpinBox->setValue(bounds->xMin());
        _ui.rightSpinBox->setValue(bounds->xMax());
        _ui.topSpinBox->setValue(bounds->yMax());
        _ui.bottomSpinBox->setValue(bounds->yMin());

        _ui.leftSpinBox->setMaximum(_ui.rightSpinBox->value());
        _ui.rightSpinBox->setMinimum(_ui.leftSpinBox->value());
        _ui.topSpinBox->setMinimum(_ui.bottomSpinBox->value());
        _ui.bottomSpinBox->setMaximum(_ui.topSpinBox->value());
    }

    auto clipSpinBoxCB = [this](double d)
    {
        _ui.openFolderButton->setEnabled(false);

        _ui.leftSpinBox->setMaximum(_ui.rightSpinBox->value());
        _ui.rightSpinBox->setMinimum(_ui.leftSpinBox->value());
        _ui.topSpinBox->setMinimum(_ui.bottomSpinBox->value());
        _ui.bottomSpinBox->setMaximum(_ui.topSpinBox->value());

        osgEarth::Bounds b(_ui.leftSpinBox->value(), _ui.bottomSpinBox->value(), _ui.rightSpinBox->value(), _ui.topSpinBox->value());

        _dataManager->setBounds(b);
        _dataManager->drawBounds(b);
    };

    connect(_ui.leftSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.rightSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.topSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.bottomSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
}

void SceneOperationsWidget::setScene(const ScenePtr& scene)
{
    if (_scene == scene)
    {
        return;
    }

    _scene = scene;
    _clipInfo.reset();

    if (!_scene)
    {
        setEnabled(false);
        return;
    }

    setEnabled(true);

    if (scene->hasScene())
    {
        _ui.statusLabel->setText(tr("Сцена находится на нашем сервере и доступна для работы"));

        _ui.importButton->setVisible(false);
        _ui.bandsGroupBox->setEnabled(true);
        _ui.boundsGroupBox->setEnabled(true);
        _ui.downloadButton->setEnabled(true);
    }
    else
    {
        if (!scene->sceneUrl().isNull() && !scene->sceneUrl().isEmpty())
        {
            _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере, но вы можете <a href='%0'>скачать</a> сцену вручную").arg(scene->sceneUrl()));
        }
        else
        {
            _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере и не доступна для работы"));
        }

        if (scene->sensor() == "Hyperion" && scene->attrib("processinglevel") == "L1T Product Available")
        {
            _ui.importButton->setVisible(true);
        }
        else
        {
            _ui.importButton->setVisible(false);
        }

        _ui.bandsGroupBox->setEnabled(false);
        _ui.boundsGroupBox->setEnabled(false);
        _ui.downloadButton->setEnabled(false);
    }

    _ui.importButton->setEnabled(!_importing);
    _ui.openFolderButton->setEnabled(false);
}
