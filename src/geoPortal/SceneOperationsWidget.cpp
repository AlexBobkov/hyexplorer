#include "SceneOperationsWidget.hpp"

#include <QDebug>
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
    _ui.globeBandSpinBox->setValue(settings.value("SceneOperationsWidget/GlobeBandValue", 1).toInt());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());

    _ui.globeBandSpinBox->setMinimum(_ui.fromSpinBox->value());
    _ui.globeBandSpinBox->setMaximum(_ui.toSpinBox->value());

    _dataManager->setActiveBand(_ui.globeBandSpinBox->value());

    _ui.statusLabel->setText(tr("Выберите сцену"));
    _ui.importButton->setVisible(false);
    _ui.bandDownloadGroupBox->setEnabled(false);
    _ui.bandOperationsGroupBox->setEnabled(false);
    _ui.processedTableButton->setEnabled(false);
        
    connect(_ui.fromSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMinimumBandChanged(int)));
    connect(_ui.toSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMaximumBandChanged(int)));
    connect(_ui.globeBandSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onGlobeBandChanged(int)));

    connect(_ui.fragmentRadioButton, SIGNAL(toggled(bool)), this, SLOT(onFragmentRadioButtonToggled(bool)));
        
    connect(_ui.selectFragmentButton, SIGNAL(toggled(bool)), this, SLOT(selectRectangle(bool)));
    connect(_ui.downloadButton, SIGNAL(clicked()), this, SLOT(download()));
    connect(_ui.importButton, SIGNAL(clicked()), this, SLOT(importScene()));
    connect(_ui.processButton, SIGNAL(clicked()), this, SLOT(startImageCorrection()));
        
    _ui.globeBandSpinBox->setValue(osg::clampBetween(_ui.globeBandSpinBox->value(), _ui.fromSpinBox->value(), _ui.toSpinBox->value()));

    boost::optional<osgEarth::Bounds> rect = _dataManager->rectangle();
    if (rect)
    {
        _ui.leftSpinBox->setValue(rect->xMin());
        _ui.rightSpinBox->setValue(rect->xMax());
        _ui.topSpinBox->setValue(rect->yMax());
        _ui.bottomSpinBox->setValue(rect->yMin());

        _ui.leftSpinBox->setMaximum(_ui.rightSpinBox->value());
        _ui.rightSpinBox->setMinimum(_ui.leftSpinBox->value());
        _ui.topSpinBox->setMinimum(_ui.bottomSpinBox->value());
        _ui.bottomSpinBox->setMaximum(_ui.topSpinBox->value());
    }
    else
    {
        _ui.leftSpinBox->setValue(-10.0);
        _ui.rightSpinBox->setValue(10.0);
        _ui.topSpinBox->setValue(10.0);
        _ui.bottomSpinBox->setValue(-10.0);
    }

    connect(_ui.leftSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRectangleBoundsChanged(double)));
    connect(_ui.rightSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRectangleBoundsChanged(double)));
    connect(_ui.topSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRectangleBoundsChanged(double)));
    connect(_ui.bottomSpinBox, SIGNAL(valueChanged(double)), this, SLOT(onRectangleBoundsChanged(double)));
}

void SceneOperationsWidget::onMinimumBandChanged(int i)
{
    QSettings settings;
    settings.setValue("SceneOperationsWidget/fromValue", _ui.fromSpinBox->value());

    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());

    _ui.globeBandSpinBox->setMinimum(_ui.fromSpinBox->value());
    _ui.globeBandSpinBox->setValue(osg::clampBetween(_ui.globeBandSpinBox->value(), _ui.fromSpinBox->value(), _ui.toSpinBox->value()));
}

void SceneOperationsWidget::onMaximumBandChanged(int i)
{
    QSettings settings;
    settings.setValue("SceneOperationsWidget/toValue", _ui.toSpinBox->value());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());

    _ui.globeBandSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.globeBandSpinBox->setValue(osg::clampBetween(_ui.globeBandSpinBox->value(), _ui.fromSpinBox->value(), _ui.toSpinBox->value()));
}

void SceneOperationsWidget::onGlobeBandChanged(int i)
{
    QSettings settings;
    settings.setValue("SceneOperationsWidget/GlobeBandValue", _ui.globeBandSpinBox->value());

    _dataManager->setActiveBand(i);
}

void SceneOperationsWidget::onFragmentRadioButtonToggled(bool b)
{
    _dataManager->setClipMode(b);
}

void SceneOperationsWidget::selectRectangle(bool b)
{
    if (b)
    {
        emit selectRectangleRequested();
    }
}

void SceneOperationsWidget::download()
{
    if (_ui.fullSizeRadioButton->isChecked())
    {
        emit downloadSceneRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value());
    }
    else
    {
        if (_dataManager->rectangle())
        {
            if (_dataManager->rectangle()->intersects(_scene->geometry->getBounds()))
            {
                emit downloadSceneClipRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value());
            }
            else
            {
                QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Фрагмент не пересекает границы сцены"));
            }
        }
        else
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Сначала выделите фрагмент"));
        }
    }
}

void SceneOperationsWidget::setScene(const ScenePtr& scene)
{
    _scene = scene;

    if (scene->hasScene)
    {
        _ui.statusLabel->setText(tr("Сцена находится на нашем сервере и доступна для работы"));

        _ui.importButton->setVisible(false);
        _ui.bandDownloadGroupBox->setEnabled(true);
        _ui.bandOperationsGroupBox->setEnabled(true);
        //_ui.processedTableButton->setEnabled(true);  
    }
    else
    {
        if (scene->sceneUrl && scene->sceneUrl->size() > 0)
        {            
            _ui.statusLabel->setText(QString::fromUtf8("Сцена отсутствует на нашем сервере, но вы можете <a href='%0'>скачать</a> сцену вручную").arg(*scene->sceneUrl));
        }
        else
        {
            _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере и не доступна для работы"));
        }
        
        if (scene->sensor == "Hyperion" && *scene->processingLevel == "L1T Product Available")
        {
            _ui.importButton->setVisible(true);
        }
        else
        {
            _ui.importButton->setVisible(false);
        }

        _ui.bandDownloadGroupBox->setEnabled(false);
        _ui.bandOperationsGroupBox->setEnabled(false);
        _ui.processedTableButton->setEnabled(false);
    }
}

void SceneOperationsWidget::onRectangleSelected(const osgEarth::Bounds& b)
{
    _ui.selectFragmentButton->setChecked(false);

    _ui.leftSpinBox->setMaximum(b.xMax());
    _ui.rightSpinBox->setMinimum(b.xMin());
    _ui.topSpinBox->setMinimum(b.yMin());
    _ui.bottomSpinBox->setMaximum(b.yMax());

    _ui.leftSpinBox->setValue(b.xMin());
    _ui.rightSpinBox->setValue(b.xMax());
    _ui.topSpinBox->setValue(b.yMax());
    _ui.bottomSpinBox->setValue(b.yMin());    
}

void SceneOperationsWidget::onRectangleSelectFailed()
{
    _ui.selectFragmentButton->setChecked(false);
}

void SceneOperationsWidget::onRectangleBoundsChanged(double d)
{
    _ui.leftSpinBox->setMaximum(_ui.rightSpinBox->value());
    _ui.rightSpinBox->setMinimum(_ui.leftSpinBox->value());
    _ui.topSpinBox->setMinimum(_ui.bottomSpinBox->value());
    _ui.bottomSpinBox->setMaximum(_ui.topSpinBox->value());
    
    emit rectangleChanged(osgEarth::Bounds(_ui.leftSpinBox->value(), _ui.bottomSpinBox->value(), _ui.rightSpinBox->value(), _ui.topSpinBox->value()));
}

void SceneOperationsWidget::importScene()
{
    _ui.importButton->setEnabled(false);

    emit importSceneRequested(_scene);
}

void SceneOperationsWidget::onSceneImported(const ScenePtr& scene)
{
    _ui.importButton->setEnabled(true);

    if (_scene == scene)
    {
        _ui.importButton->setVisible(false);
    }
}

void SceneOperationsWidget::startImageCorrection()
{
    qDebug() << "Image correction started";

    _ui.processButton->setEnabled(false);

    QString program = "matlab/ImageCorrectionTools.exe";
    QStringList arguments;
    
    QProcess* matlabProcess = new QProcess(this);
    matlabProcess->setWorkingDirectory("matlab");

    connect(matlabProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onImageCorrectionError(QProcess::ProcessError)));
    connect(matlabProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onImageCorrectionFinished(int, QProcess::ExitStatus)));

    matlabProcess->start(program, arguments);
}

void SceneOperationsWidget::onImageCorrectionError(QProcess::ProcessError error)
{
    qDebug() << "Image correnction failed. Error code" << error;

    _ui.processButton->setEnabled(true);

    QProcess* process = qobject_cast<QProcess*>(sender());
    process->deleteLater();

    QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Ошибка при выполнении обработки. Код %0").arg(error));
}

void SceneOperationsWidget::onImageCorrectionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Image correnction finished. Exit code" << exitCode << "exit status" << exitStatus;

    _ui.processButton->setEnabled(true);

    QProcess* process = qobject_cast<QProcess*>(sender());    
    process->deleteLater();

    QMessageBox::information(qApp->activeWindow(), tr("Обработка"), tr("Обработка завершена. Обработанный файл будет загружен на сервер"));    
}