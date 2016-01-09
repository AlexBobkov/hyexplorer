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
#include <QHttpMultiPart>

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

    _ui.importButton->setVisible(false);
    _ui.bandDownloadGroupBox->setEnabled(false);
    _ui.bandOperationsGroupBox->setEnabled(false);
    _ui.processedTableButton->setEnabled(false);

    //---------------------------------------

    QSettings settings;
    _ui.fromSpinBox->setValue(settings.value("SceneOperationsWidget/fromValue", 1).toInt());
    _ui.toSpinBox->setValue(settings.value("SceneOperationsWidget/toValue", 1).toInt());
    _ui.globeBandSpinBox->setValue(settings.value("SceneOperationsWidget/GlobeBandValue", 1).toInt());

    _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
    _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());

    _ui.globeBandSpinBox->setMinimum(_ui.fromSpinBox->value());
    _ui.globeBandSpinBox->setMaximum(_ui.toSpinBox->value());
        
    connect(_ui.fromSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("SceneOperationsWidget/fromValue", _ui.fromSpinBox->value());

        _ui.toSpinBox->setMinimum(_ui.fromSpinBox->value());
        _ui.globeBandSpinBox->setMinimum(_ui.fromSpinBox->value());
    });

    connect(_ui.toSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("SceneOperationsWidget/toValue", _ui.toSpinBox->value());

        _ui.fromSpinBox->setMaximum(_ui.toSpinBox->value());
        _ui.globeBandSpinBox->setMaximum(_ui.toSpinBox->value());
    });

    connect(_ui.globeBandSpinBox, static_cast<void(QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, [this](int i)
    {
        QSettings settings;
        settings.setValue("SceneOperationsWidget/GlobeBandValue", _ui.globeBandSpinBox->value());
    });

    //---------------------------------------

    connect(_ui.selectFragmentButton, SIGNAL(clicked()), this, SIGNAL(selectRectangleRequested()));
    connect(_ui.downloadButton, &QPushButton::clicked, this, &SceneOperationsWidget::download);
    connect(_ui.processButton, &QPushButton::clicked, this, &SceneOperationsWidget::startImageCorrection);
    connect(_ui.openFolderButton, &QPushButton::clicked, this, &SceneOperationsWidget::openFolder);
    connect(_ui.processedTableButton, &QPushButton::clicked, this, &SceneOperationsWidget::showTableWithProcessedFiles);

    connect(_ui.importButton, &QPushButton::clicked, this, [this]()
    {
        _ui.importButton->setVisible(false);

        emit importSceneRequested(_scene);
    });    

    connect(_ui.showOnGlobeButton, &QPushButton::clicked, this, [this]()
    {
        if (_ui.fullSizeRadioButton->isChecked())
        {
            _dataManager->showScene(_scene, _ui.globeBandSpinBox->value(), ClipInfoPtr());
        }
        else
        {
            _dataManager->showScene(_scene, _ui.globeBandSpinBox->value(), _dataManager->clipInfo());
        }
    });    

    //---------------------------------------

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

    auto clipSpinBoxCB = [this](double d)
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
    };

    connect(_ui.leftSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.rightSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.topSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
    connect(_ui.bottomSpinBox, static_cast<void(QDoubleSpinBox::*)(double)>(&QDoubleSpinBox::valueChanged), this, clipSpinBoxCB);
}

void SceneOperationsWidget::download()
{
    if (_ui.fullSizeRadioButton->isChecked())
    {
        _ui.downloadButton->setEnabled(false);

        emit downloadSceneRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value(), ClipInfoPtr());
    }
    else
    {
        if (_dataManager->clipInfo())
        {
            if (_dataManager->clipInfo()->bounds().intersects(_scene->geometry()->getBounds()))
            {
                _ui.downloadButton->setEnabled(false);

                emit downloadSceneRequested(_scene, _ui.fromSpinBox->value(), _ui.toSpinBox->value(), _dataManager->clipInfo());
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

        openFolder();
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

    if (scene->hasScene())
    {
        _ui.statusLabel->setText(tr("Сцена находится на нашем сервере и доступна для работы"));

        _ui.importButton->setVisible(false);
        _ui.bandDownloadGroupBox->setEnabled(true);
        _ui.bandOperationsGroupBox->setEnabled(true);
        _ui.processedTableButton->setEnabled(true);  
    }
    else
    {
        if (!scene->sceneUrl().isNull() && !scene->sceneUrl().isEmpty())
        {
            _ui.statusLabel->setText(QString::fromUtf8("Сцена отсутствует на нашем сервере, но вы можете <a href='%0'>скачать</a> сцену вручную").arg(scene->sceneUrl()));
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

    connect(matlabProcess, static_cast<void(QProcess::*)(QProcess::ProcessError)>(&QProcess::error), this, [matlabProcess, this](QProcess::ProcessError error)
    {
        qDebug() << "Image correnction failed. Error code" << error;

        matlabProcess->deleteLater();

        _ui.processButton->setEnabled(true);

        _proccessedOutputFilepath.clear();
        _processingScene.reset();

        QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Ошибка при выполнении обработки. Код %0").arg(error));
    });

    connect(matlabProcess, static_cast<void(QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished), this, [matlabProcess, this](int exitCode, QProcess::ExitStatus exitStatus)
    {
        qDebug() << "Image correnction finished. Exit code" << exitCode << "exit status" << exitStatus;
                
        matlabProcess->deleteLater();

        if (QFileInfo::exists(_proccessedOutputFilepath))
        {
            openExplorer(_proccessedOutputFilepath);

            uploadProccessedFile();
        }
        else
        {
            _ui.processButton->setEnabled(true);

            _proccessedOutputFilepath.clear();
            _processingScene.reset();

            QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Обработка прервана пользователем"));
        }
    });

    matlabProcess->start(program, QStringList());
}

void SceneOperationsWidget::uploadProccessedFile()
{
    qDebug() << "Upload " << _proccessedOutputFilepath;

    double contrast = 123.0;
    double sharpness = 45.0;
    int blocksize = 666;

    //------------------------------------

    QFileInfo fileInfo(_proccessedOutputFilepath);

    QHttpMultiPart* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    //------------------------------------

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader, QString("application/octet-stream"));
    imagePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"file\";filename=\"%0\"").arg(fileInfo.fileName()));

    QFile* file = new QFile(_proccessedOutputFilepath);
    file->open(QIODevice::ReadOnly);
    imagePart.setBodyDevice(file);
    file->setParent(multiPart); // we cannot delete the file now, so delete it with the multiPart

    multiPart->append(imagePart);

    //------------------------------------

    QHttpPart bandPart;
    bandPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"band\""));
    bandPart.setBody(QByteArray::number(_processingBand));

    multiPart->append(bandPart);

    //------------------------------------

    QHttpPart contrastPart;
    contrastPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"contrast\""));
    contrastPart.setBody(QByteArray::number(contrast, 'f', 7));

    multiPart->append(contrastPart);

    //------------------------------------

    QHttpPart sharpnessPart;
    sharpnessPart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"sharpness\""));
    sharpnessPart.setBody(QByteArray::number(sharpness, 'f', 7));

    multiPart->append(sharpnessPart);

    //------------------------------------

    QHttpPart blocksizePart;
    blocksizePart.setHeader(QNetworkRequest::ContentDispositionHeader, QString("form-data;name=\"blocksize\""));
    blocksizePart.setBody(QByteArray::number(blocksize));

    multiPart->append(blocksizePart);

    //------------------------------------

    QUrl url = QString("http://virtualglobe.ru/geoportalapi/processed/%0").arg(_processingScene->sceneId());

    QNetworkReply* reply = _dataManager->networkAccessManager().post(QNetworkRequest(url), multiPart);

    multiPart->setParent(reply); // delete the multiPart with the reply

    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        reply->deleteLater();

        _ui.processButton->setEnabled(true);

        _proccessedOutputFilepath.clear();
        _processingScene.reset();

        if (reply->error() != QNetworkReply::NoError)
        {
            qDebug() << "Error " << reply->error() << " " << reply->errorString();            
            QMessageBox::warning(qApp->activeWindow(), tr("Обработка"), tr("Ошибка при загрузке файла на сервер %1 %2").arg(reply->error()).arg(reply->errorString()));
            return;
        }

        QMessageBox::information(qApp->activeWindow(), tr("Обработка"), tr("Обработка завершена. Обработанный файл был загружен на сервер"));
    });
}

void SceneOperationsWidget::showTableWithProcessedFiles()
{
    QDialog tableWindow;

    QVBoxLayout* layout = new QVBoxLayout;
    
    QSqlQueryModel* model = new QSqlQueryModel(&tableWindow);
    model->setQuery(QString("SELECT band, contrast, sharpness, blocksize, filename FROM public.processedimages where sceneid='%0'").arg(_scene->sceneId()));    
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

    connect(button, &QPushButton::clicked, this, [button, this, view, model]()
    {
        if (!view->currentIndex().isValid())
        {
            QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Выберите строчку в таблице"));
            return;
        }

        int row = view->currentIndex().row();
        QString filename = model->record(row).value("filename").toString();

        downloadProcessedFile(filename);
    });

    tableWindow.setLayout(layout);
    tableWindow.setWindowTitle(tr("Обработанные файлы для сцены %0").arg(_scene->sceneId()));
    tableWindow.resize(600, 300);
    tableWindow.exec();
}

void SceneOperationsWidget::downloadProcessedFile(const QString& filename)
{
    QString filepath = Storage::processedFileDir(_scene).filePath(filename);
    if (QFileInfo::exists(filepath))
    {
        QMessageBox::warning(qApp->activeWindow(), tr("Предупреждение"), tr("Файл уже получен"));

        openExplorer(filepath);
        return;
    }

    QUrl url = QString("http://virtualglobe.ru/geoportal/Hyperion/scenes/%0/processed/%1").arg(_scene->sceneId()).arg(filename);
                
    QNetworkReply* reply = _dataManager->networkAccessManager().get(QNetworkRequest(url));
    connect(reply, &QNetworkReply::finished, this, [reply, this]()
    {
        reply->deleteLater();

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