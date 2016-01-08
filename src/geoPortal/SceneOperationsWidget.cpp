#include "SceneOperationsWidget.hpp"
#include "Storage.hpp"

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

using namespace portal;

namespace
{
    void openExplorer(const QString& path)
    {
        if (QFileInfo(path).isDir())
        {
            QDesktopServices::openUrl(QUrl(QString("file:///") + path.toLocal8Bit()));
        }
        else
        {            
            QProcess::startDetached(QString("explorer.exe /select,\"%0\"").arg(QDir::toNativeSeparators(path.toLocal8Bit())));
        }
    }
}

SceneOperationsWidget::SceneOperationsWidget(const DataManagerPtr& dataManager, QWidget* parent) :
QWidget(parent),
_dataManager(dataManager),
_processingBand(-1)
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
    connect(_ui.showOnGlobeButton, SIGNAL(clicked()), this, SLOT(showBandOnGlobe()));
    connect(_ui.openFolderButton, SIGNAL(clicked()), this, SLOT(openFolder()));
    connect(_ui.processedTableButton, SIGNAL(clicked()), this, SLOT(showTableWithProcessedFiles()));

    _ui.globeBandSpinBox->setValue(osg::clampBetween(_ui.globeBandSpinBox->value(), _ui.fromSpinBox->value(), _ui.toSpinBox->value()));

    ClipInfoPtr clipInfo = _dataManager->clipInfo();
    if (clipInfo)
    {
        _ui.leftSpinBox->setValue(clipInfo->bounds().xMin());
        _ui.rightSpinBox->setValue(clipInfo->bounds().xMax());
        _ui.topSpinBox->setValue(clipInfo->bounds().yMax());
        _ui.bottomSpinBox->setValue(clipInfo->bounds().yMin());

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
}

void SceneOperationsWidget::onFragmentRadioButtonToggled(bool b)
{
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
        _ui.downloadButton->setEnabled(false);

        emit downloadSceneRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value());
    }
    else
    {
        if (_dataManager->clipInfo())
        {
            if (_dataManager->clipInfo()->bounds().intersects(_scene->geometry->getBounds()))
            {
                _ui.downloadButton->setEnabled(false);

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

void SceneOperationsWidget::onSceneDownloaded(const ScenePtr& scene, bool result, const QString& message)
{
    _ui.downloadButton->setEnabled(true);

    if (result)
    {
        _ui.openFolderButton->setEnabled(true);

        QMessageBox::information(qApp->activeWindow(), tr("Выбранные каналы получены"), message);
    }
    else
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Ошибка получения сцены"), message);
    }
}

void SceneOperationsWidget::setScene(const ScenePtr& scene)
{
    if (_scene == scene)
    {
        return;
    }

    _scene = scene;

    if (scene->hasScene)
    {
        _ui.statusLabel->setText(tr("Сцена находится на нашем сервере и доступна для работы"));

        _ui.importButton->setVisible(false);
        _ui.bandDownloadGroupBox->setEnabled(true);
        _ui.bandOperationsGroupBox->setEnabled(true);
        _ui.processedTableButton->setEnabled(true);  
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

    _ui.openFolderButton->setEnabled(false);
}

void SceneOperationsWidget::onRectangleSelected(const osgEarth::Bounds& b)
{
    _ui.selectFragmentButton->setChecked(false);
    _ui.openFolderButton->setEnabled(false);

    _ui.leftSpinBox->setMaximum(b.xMax());
    _ui.rightSpinBox->setMinimum(b.xMin());
    _ui.topSpinBox->setMinimum(b.yMin());
    _ui.bottomSpinBox->setMaximum(b.yMax());

    _ui.leftSpinBox->setValue(b.xMin());
    _ui.rightSpinBox->setValue(b.xMax());
    _ui.topSpinBox->setValue(b.yMax());
    _ui.bottomSpinBox->setValue(b.yMin());

    ClipInfoPtr clipInfo = std::make_shared<ClipInfo>(b);
    clipInfo->setMinBand(_ui.fromSpinBox->value());
    clipInfo->setMaxBand(_ui.toSpinBox->value());

    _dataManager->setClipInfo(clipInfo);
}

void SceneOperationsWidget::onRectangleSelectFailed()
{
    _ui.selectFragmentButton->setChecked(false);
}

void SceneOperationsWidget::onRectangleBoundsChanged(double d)
{
    _ui.openFolderButton->setEnabled(false);

    _ui.leftSpinBox->setMaximum(_ui.rightSpinBox->value());
    _ui.rightSpinBox->setMinimum(_ui.leftSpinBox->value());
    _ui.topSpinBox->setMinimum(_ui.bottomSpinBox->value());
    _ui.bottomSpinBox->setMaximum(_ui.topSpinBox->value());

    osgEarth::Bounds b(_ui.leftSpinBox->value(), _ui.bottomSpinBox->value(), _ui.rightSpinBox->value(), _ui.topSpinBox->value());

    ClipInfoPtr clipInfo = std::make_shared<ClipInfo>(b);
    clipInfo->setMinBand(_ui.fromSpinBox->value());
    clipInfo->setMaxBand(_ui.toSpinBox->value());

    _dataManager->setClipInfo(clipInfo);

    emit rectangleChanged(b);
}

void SceneOperationsWidget::importScene()
{
    _ui.importButton->setVisible(false);

    emit importSceneRequested(_scene);
}

void SceneOperationsWidget::onSceneImported(const ScenePtr& scene)
{
}

void SceneOperationsWidget::startImageCorrection()
{
    if (_processingScene)
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Дождитесь завершения обработки предыдущей сцены"));
        return;
    }

    QString program = "matlab/ImageCorrectionTool.exe";

    if (!QFileInfo::exists(program))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Программа для коррекции изображений matlab/ImageCorrectionTool.exe не найдена"));
        return;
    }

    QString filepath;
    if (_ui.fullSizeRadioButton->isChecked())
    {
        filepath = Storage::sceneBandPath(_scene, _ui.globeBandSpinBox->value());
    }
    else
    {
        filepath = Storage::sceneBandPath(_scene, _ui.globeBandSpinBox->value(), _dataManager->clipInfo());
    }

    if (!QFileInfo::exists(filepath))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Файл для обработки %0 не найден").arg(filepath));
        return;
    }

    //------------------------------------

    {
        QFile data("matlab/data.txt");
        data.open(QFile::WriteOnly);

        QTextStream out(&data);
        out << filepath.toLocal8Bit() << "\n" << 11 << "\n" << 1.4 << "\n" << 128 << "\n";

        data.close();
    }

    //------------------------------------

    {
        _proccessedOutputFilepath = Storage::processedFilePath(_scene, _ui.globeBandSpinBox->value(), genetrateRandomName());

        QFile result("matlab/result.txt");
        result.open(QFile::WriteOnly);

        QTextStream out(&result);
        out << _proccessedOutputFilepath.toLocal8Bit() << "\n";

        result.close();
    }

    //------------------------------------

    _processingScene = _scene;
    _processingBand = _ui.globeBandSpinBox->value();

    qDebug() << "Image correction started";

    _ui.processButton->setEnabled(false);

    QProcess* matlabProcess = new QProcess(this);
    matlabProcess->setWorkingDirectory("matlab");

    connect(matlabProcess, SIGNAL(error(QProcess::ProcessError)), this, SLOT(onImageCorrectionError(QProcess::ProcessError)));
    connect(matlabProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(onImageCorrectionFinished(int, QProcess::ExitStatus)));

    matlabProcess->start(program, QStringList());
}

void SceneOperationsWidget::onImageCorrectionError(QProcess::ProcessError error)
{
    qDebug() << "Image correnction failed. Error code" << error;

    _ui.processButton->setEnabled(true);

    QProcess* process = qobject_cast<QProcess*>(sender());
    process->deleteLater();

    _proccessedOutputFilepath.clear();
    _processingScene.reset();

    QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Ошибка при выполнении обработки. Код %0").arg(error));
}

void SceneOperationsWidget::onImageCorrectionFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    qDebug() << "Image correnction finished. Exit code" << exitCode << "exit status" << exitStatus;

    QProcess* process = qobject_cast<QProcess*>(sender());
    process->deleteLater();

    if (!QFileInfo::exists(_proccessedOutputFilepath))
    {
        _ui.processButton->setEnabled(true);

        _proccessedOutputFilepath.clear();
        _processingScene.reset();

        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Обработка прервана пользователем"));

        return;
    }

    uploadProccessedFile();
}

void SceneOperationsWidget::uploadProccessedFile()
{
    assert(!_proccessedOutputFilepath.isNull() && !_proccessedOutputFilepath.isEmpty());
    assert(QFileInfo::exists(_proccessedOutputFilepath));

    emit uploadProcessedFileRequested(_processingScene, _proccessedOutputFilepath, _processingBand, 123.0, 45.0, 666);
}

void SceneOperationsWidget::onProcessedFileUploaded(const ScenePtr& scene, bool result, const QString& message)
{
    _ui.processButton->setEnabled(true);

    _proccessedOutputFilepath.clear();
    _processingScene.reset();

    if (result)
    {
        QMessageBox::information(qApp->activeWindow(), tr("Обработка"), tr("Обработка завершена. Обработанный файл был загружен на сервер"));
    }
    else
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Ошибка при загрузке файла на сервер. Сообщение: %0").arg(message));
    }
}

void SceneOperationsWidget::openFolder()
{
    if (_ui.fullSizeRadioButton->isChecked())
    {
        openExplorer(Storage::sceneBandDir(_scene).path());
    }
    else
    {
        if (_dataManager->clipInfo())
        {
            openExplorer(Storage::sceneBandDir(_scene, _dataManager->clipInfo()).path());
        }
    }
}

void SceneOperationsWidget::showBandOnGlobe()
{
    if (_ui.fullSizeRadioButton->isChecked())
    {
        _dataManager->showScene(_scene, _ui.globeBandSpinBox->value(), ClipInfoPtr());
    }
    else
    {
        _dataManager->showScene(_scene, _ui.globeBandSpinBox->value(), _dataManager->clipInfo());
    }
}

void SceneOperationsWidget::showTableWithProcessedFiles()
{
    QDialog tableWindow;

    QVBoxLayout* layout = new QVBoxLayout;
    
    QSqlQueryModel* model = new QSqlQueryModel(&tableWindow);
    model->setQuery(QString("SELECT band, contrast, sharpness, blocksize, filename FROM public.processedimages where sceneid='%0'").arg(_scene->sceneId));    
    model->setHeaderData(0, Qt::Horizontal, tr("Канал"));
    model->setHeaderData(1, Qt::Horizontal, tr("Контраст"));
    model->setHeaderData(2, Qt::Horizontal, tr("Резкость"));
    model->setHeaderData(3, Qt::Horizontal, tr("Размер блока"));

    QTableView* view = new QTableView(&tableWindow);    
    view->setModel(model);
    view->resizeColumnToContents(4);
    layout->addWidget(view);
        
    QPushButton* button = new QPushButton(tr("Скачать"), &tableWindow);
    button->setMaximumWidth(100);
    layout->addWidget(button);
    connect(button, SIGNAL(clicked()), this, SLOT(downloadProcessedFile()));

    tableWindow.setLayout(layout);
    tableWindow.setWindowTitle(tr("Обработанные файлы для сцены %0").arg(_scene->sceneId));
    tableWindow.resize(600, 300);
    tableWindow.exec();
}

void SceneOperationsWidget::downloadProcessedFile()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    assert(button);

    QDialog* tableWindow = qobject_cast<QDialog*>(button->parent());
    assert(tableWindow);

    QTableView* view = qobject_cast<QTableView*>(tableWindow->layout()->itemAt(0)->widget());
    assert(view);
        
    QSqlQueryModel* model = qobject_cast<QSqlQueryModel*>(view->model());
    assert(model);

    if (!view->currentIndex().isValid())
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Выберите строчку в таблице"));
        return;
    }

    int row = view->currentIndex().row();    
    QString filename = model->record(row).value("filename").toString();
    QString filepath = Storage::processedFileDir(_scene).filePath(filename);

    if (QFileInfo::exists(filepath))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Файл уже получен"));

        openExplorer(filepath);

        return;
    }

    QUrl url = QString("http://virtualglobe.ru/geoportal/Hyperion/scenes/processed/%0/%1").arg(_scene->sceneId).arg(filename);
                
    QNetworkReply* reply = _dataManager->networkAccessManager().get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Error " << reply->error() << " " << reply->errorString();
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Не удалось скачать файл. Ошибка %0 %1").arg(reply->error()).arg(reply->errorString()));
            return;
        }

        QByteArray data = reply->readAll();
        if (data.isNull() || data.isEmpty())
        {
            qDebug() << "Reply is null or empty";
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Не удалось скачать файл. Пустой ответ от сервера"));
            return;
        }

        QString path = Storage::processedFileDir(_scene).filePath(reply->url().fileName());

        QFile localFile(path);
        localFile.open(QIODevice::WriteOnly);
        localFile.write(data);
        localFile.close();

        QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Файл был успешно получен"));

        openExplorer(path);
    });
}