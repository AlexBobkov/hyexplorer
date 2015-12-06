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
    _ui.globeBandSpinBox->setValue(settings.value("SceneOperationsWidget/BandValue", 1).toInt());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());

    _ui.globeBandSpinBox->setMinimum(_ui.fromSpinBox->value());
    _ui.globeBandSpinBox->setMaximum(_ui.toSpinBox->value());

    _dataManager->setActiveBand(_ui.globeBandSpinBox->value());

    _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере\nи не доступна для работы"));
    _ui.getFromUsgsButton->setVisible(false);
    _ui.bandsGroupBox->setVisible(false);
    _ui.boundsGroupBox->setVisible(false);
    _ui.downloadButton->setVisible(false);
        
    connect(_ui.fromSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMinimumBandChanged(int)));
    connect(_ui.toSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onMaximumBandChanged(int)));
    connect(_ui.globeBandSpinBox, SIGNAL(valueChanged(int)), this, SLOT(onGlobeBandChanged(int)));

    connect(_ui.fragmentRadioButton, SIGNAL(toggled(bool)), this, SLOT(onFragmentRadioButtonToggled(bool)));
        
    connect(_ui.selectFragmentButton, SIGNAL(toggled(bool)), this, SLOT(selectRectangle(bool)));
    connect(_ui.downloadButton, SIGNAL(clicked()), this, SLOT(download()));
    connect(_ui.getFromUsgsButton, SIGNAL(clicked()), this, SLOT(getFromUsgs()));
        
    _ui.globeBandSpinBox->setValue(osg::clampBetween(_ui.globeBandSpinBox->value(), _ui.fromSpinBox->value(), _ui.toSpinBox->value()));

    boost::optional<osgEarth::Bounds> rect = _dataManager->rectangle();
    if (rect)
    {
        _ui.leftSpinBox->setValue(rect->xMin());
        _ui.rightSpinBox->setValue(rect->xMax());
        _ui.topSpinBox->setValue(rect->yMax());
        _ui.bottomSpinBox->setValue(rect->yMin());
    }
    else
    {
        _ui.leftSpinBox->setValue(0.0);
        _ui.rightSpinBox->setValue(0.0);
        _ui.topSpinBox->setValue(0.0);
        _ui.bottomSpinBox->setValue(0.0);
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
    settings.setValue("SceneOperationsWidget/BandValue", _ui.globeBandSpinBox->value());

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
            emit downloadSceneClipRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value());
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
        _ui.statusLabel->setText(tr("Сцена присутствует на нашем сервере\nи доступна для скачивания"));
        _ui.getFromUsgsButton->setVisible(false);
        _ui.bandsGroupBox->setVisible(true);
        _ui.boundsGroupBox->setVisible(true);
        _ui.downloadButton->setVisible(true);        
    }
    else
    {
        _ui.statusLabel->setText(tr("Сцена отсутствует на нашем сервере\nи не доступна для работы"));
        _ui.getFromUsgsButton->setVisible(true);
        _ui.bandsGroupBox->setVisible(false);
        _ui.boundsGroupBox->setVisible(false);
        _ui.downloadButton->setVisible(false);
    }
}

void SceneOperationsWidget::onRectangleSelected(const osgEarth::Bounds& b)
{
    _ui.selectFragmentButton->setChecked(false);

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
    emit rectangleChanged(osgEarth::Bounds(_ui.leftSpinBox->value(), _ui.bottomSpinBox->value(), _ui.rightSpinBox->value(), _ui.topSpinBox->value()));
}

void SceneOperationsWidget::getFromUsgs()
{
    _ui.getFromUsgsButton->setEnabled(false);

    emit getFromUsgsRequested(_scene);
}

void SceneOperationsWidget::onSceneGotFromUsgs(const ScenePtr& scene)
{
    _ui.getFromUsgsButton->setEnabled(true);

    if (_scene == scene)
    {
        _ui.getFromUsgsButton->setVisible(false);
    }
}